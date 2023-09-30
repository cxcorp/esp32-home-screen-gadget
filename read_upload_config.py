import json

Import("env")

with open("./upload_config.json", "r") as f:
    config = json.load(f)
    env.Append(
        UPLOAD_FLAGS=[
            "--auth",
            config["esp_device_ota_auth"],
            "--port",
            config["esp_device_ota_port"],
        ]
    )
    env.Replace(UPLOAD_PORT=config["esp_device_ota_ip"])
