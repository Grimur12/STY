//
//  testfile.swift
//  Audkenni
//

import SwiftUI

struct ResultsView: View {
    let image: UIImage
    let fev1: Double
    let fvc: Double

    var body: some View {
        VStack(spacing: 20) {
            Text("Niðurstöður")
                .font(.title)
                .bold()

            Image(uiImage: image)
                .resizable()
                .scaledToFit()
                .frame(height: 250)
                .clipShape(RoundedRectangle(cornerRadius: 12))
                .shadow(radius: 4)

            VStack(spacing: 10) {
                Text("FEV1: \(String(format: "%.2f", fev1)) L")
                    .font(.title3)
                    .fontWeight(.medium)

                Text("FVC: \(String(format: "%.2f", fvc)) L")
                    .font(.title3)
                    .fontWeight(.medium)
            }

            Spacer()
        }
        .padding()
    }
}
#Preview {
    ResultsView(image: UIImage(systemName: "photo")!, fev1: 100, fvc: 200)
}
