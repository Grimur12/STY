import SwiftUI
import AVFoundation
import Foundation

struct NewRecordingView: View {
    @StateObject private var audioRecorder = AudioRecorder()
    @State private var navigateToResults = false
    @State private var resultImage: UIImage?
    @State private var fev1: Double = 0.0
    @State private var fvc: Double = 0.0

    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                // Balloon Box
                RoundedRectangle(cornerRadius: 10)
                    .strokeBorder(Color.gray, lineWidth: 2)
                    .frame(height: 150)
                    .overlay(
                        VStack {
                            if audioRecorder.isRecording {
                                Text("🎈")
                                    .font(.system(size: 20 + CGFloat(audioRecorder.currentLevel) * 120))
                                    .animation(.easeOut(duration: 0.1), value: audioRecorder.currentLevel)
                            } else {
                                Text("Leiðbeiningar hér")
                                    .font(.body)
                                    .foregroundColor(.gray)
                            }
                        }
                    )

                // Waveform Box
                RoundedRectangle(cornerRadius: 10)
                    .strokeBorder(Color.gray, lineWidth: 2)
                    .frame(height: 80)
                    .overlay(
                        Group {
                            if audioRecorder.isRecording {
                                WaveformView(samples: audioRecorder.waveformSamples)
                                    .padding(.horizontal)
                            }
                        }
                    )

                // Record Button
                Button(action: {
                    if audioRecorder.isRecording {
                        audioRecorder.stopRecording()
                        audioRecorder.uploadToLambda { image, fev1, fvc in
                            self.resultImage = image
                            self.fev1 = fev1
                            self.fvc = fvc
                            self.navigateToResults = true
                        }
                    } else {
                        audioRecorder.startRecording()
                    }
                }) {
                    HStack {
                        Image(systemName: audioRecorder.isRecording ? "stop.circle.fill" : "play.circle.fill")
                            .font(.largeTitle)
                            .foregroundColor(.white)
                        Text(audioRecorder.isRecording ? "Upptaka í gangi..." : "Byrja upptöku")
                            .font(.title3)
                            .fontWeight(.semibold)
                            .foregroundColor(.white)
                    }
                    .padding()
                    .frame(maxWidth: .infinity)
                    .background(audioRecorder.isRecording ? Color.red : Color.blue)
                    .cornerRadius(10)
                    .animation(.easeInOut, value: audioRecorder.isRecording)
                }

                // Navigation to results
                NavigationLink("", destination:
                    VStack(spacing: 20) {
                        Text("Niðurstöður")
                            .font(.title).bold()
                        if let image = resultImage {
                            Image(uiImage: image)
                                .resizable()
                                .scaledToFit()
                                .frame(height: 250)
                                .clipShape(RoundedRectangle(cornerRadius: 12))
                                .shadow(radius: 4)
                        }
                        Text("FEV1: \(String(format: "%.2f", fev1)) L")
                        Text("FVC: \(String(format: "%.2f", fvc)) L")
                        Spacer()
                    }
                    .padding()
                , isActive: $navigateToResults)
            }
            .padding()
            .navigationBarTitleDisplayMode(.inline)
        }
    }
}

struct WaveformView: View {
    var samples: [Float]

    var body: some View {
        GeometryReader { geometry in
            VStack {
                HStack(alignment: .center, spacing: 2) {
                    ForEach(samples.indices, id: \.self) { i in
                        let height = max(1, CGFloat(samples[i]) * geometry.size.height)
                        Capsule()
                            .frame(width: 3, height: height)
                            .foregroundColor(.blue)
                    }
                }
                .frame(maxHeight: .infinity, alignment: .center)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }
}

class AudioRecorder: NSObject, ObservableObject, AVAudioRecorderDelegate {
    var audioRecorder: AVAudioRecorder?
    var meterTimer: Timer?

    @Published var isRecording = false
    @Published var audioFileURL: URL?
    @Published var currentLevel: Float = 0.0
    @Published var waveformSamples: [Float] = []

    func startRecording() {
        let fileName = UUID().uuidString + ".wav"
        let path = FileManager.default.temporaryDirectory.appendingPathComponent(fileName)

        do {
            let session = AVAudioSession.sharedInstance()
            try session.setCategory(.playAndRecord, mode: .default, options: [.defaultToSpeaker])
            try session.setActive(true)

            let settings: [String: Any] = [
                AVFormatIDKey: Int(kAudioFormatLinearPCM),
                AVSampleRateKey: 44100,
                AVNumberOfChannelsKey: 1,
                AVLinearPCMBitDepthKey: 16,
                AVLinearPCMIsFloatKey: false,
                AVLinearPCMIsBigEndianKey: false
            ]


            audioRecorder = try AVAudioRecorder(url: path, settings: settings)
            audioRecorder?.delegate = self
            audioRecorder?.isMeteringEnabled = true
            audioRecorder?.record()

            isRecording = true
            audioFileURL = path
            waveformSamples = []

            meterTimer = Timer.scheduledTimer(withTimeInterval: 0.05, repeats: true) { _ in
                self.audioRecorder?.updateMeters()
                if let power = self.audioRecorder?.averagePower(forChannel: 0) {
                    let level = max(0.0, min(1.0, (power + 60) / 60))
                    DispatchQueue.main.async {
                        self.currentLevel = level
                        self.waveformSamples.append(level)
                        if self.waveformSamples.count > 60 {
                            self.waveformSamples.removeFirst()
                        }
                    }
                }
            }

        } catch {
            print("Failed to start recording: \(error.localizedDescription)")
        }
    }

    func stopRecording() {
        audioRecorder?.stop()
        isRecording = false
        audioFileURL = audioRecorder?.url
        meterTimer?.invalidate()
        meterTimer = nil
        currentLevel = 0.0
    }

    func uploadToLambda(completion: @escaping (UIImage?, Double, Double) -> Void) {
        guard let fileURL = audioFileURL else { return }

        do {
            let audioData = try Data(contentsOf: fileURL)
            let base64String = audioData.base64EncodedString()

            let payload: [String: Any] = [
                "body": base64String
            ]

            let jsonData = try JSONSerialization.data(withJSONObject: payload)
            let url = URL(string: "")!

            var request = URLRequest(url: url)
            request.httpMethod = "POST"
            request.setValue("application/json", forHTTPHeaderField: "Content-Type")
            request.httpBody = jsonData

            URLSession.shared.dataTask(with: request) { data, response, error in
                if let error = error {
                    print("Error uploading:", error)
                    return
                }

                guard let data = data else {
                    print("No data in response")
                    return
                }

                // Try parsing the response
                do {
                    let json = try JSONSerialization.jsonObject(with: data, options: []) as? [String: Any]

                    if let fev1 = json?["FEV1"] as? Double,
                       let fvc = json?["FVC"] as? Double,
                       let imageBase64 = json?["image"] as? String,
                       let imageData = Data(base64Encoded: imageBase64),
                       let image = UIImage(data: imageData) {

                        DispatchQueue.main.async {
                            completion(image, fev1, fvc)
                        }

                    } else {
                        print("Incomplete or unexpected response:")
                        print("JSON:", json ?? "nil")
                    }
                } catch {
                    print("JSON parsing failed")
                    print("Raw response string:")
                    print(String(data: data, encoding: .utf8) ?? "Unreadable data")
                }

            }.resume()

        } catch {
            print("Failed to upload file:", error)
        }
    }
}

#Preview {
    NewRecordingView()
}
