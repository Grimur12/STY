import base64
import json
import uuid

def fix_base64_padding(b64_string):
    return b64_string + "=" * ((4 - len(b64_string) % 4) % 4)




def lambda_handler(event, context):
    try:
        # Always base64 decode
        base64_str = event["body"]
        base64_str = fix_base64_padding(base64_str)
        audio_data = base64.b64decode(base64_str)

        # Save to file
        filename = f"/tmp/{uuid.uuid4().hex}.wav"
        with open(filename, "wb") as f:
            f.write(audio_data)

        # Mock response
        return {
            "statusCode": 200,
            "body": json.dumps({
                "FEV1": 3.2,
                "FVC": 4.1,
                "image": base64.b64encode(b"fakeImageBytes").decode("utf-8")
            }),
            "headers": {
                "Content-Type": "application/json"
            }
        }

    except Exception as e:
        return {
            "statusCode": 500,
            "body": str(e)
        }
