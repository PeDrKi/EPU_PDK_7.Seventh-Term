import requests
import csv
import time
from datetime import datetime

# ----------------------------
# Cấu hình của bạn
# ----------------------------
CHANNEL_ID = "3126438"              
READ_API_KEY = "8V963RSF531QS9V0"
FILENAME = "data.csv" 
INTERVAL = 15                     
# ----------------------------

# URL đọc dữ liệu (định dạng JSON để dễ xử lý)
url = f"https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds.json?api_key={READ_API_KEY}&results=1"

# Tạo file CSV (nếu chưa có)
with open(FILENAME, mode='w', newline='', encoding='utf-8') as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp", "temperature", "humidity", "mq9"])

print("Bắt đầu ghi dữ liệu ThingSpeak mỗi 15s...")
while True:
    try:
        response = requests.get(url)
        if response.status_code == 200:
            data = response.json()
            feeds = data.get("feeds", [])
            if len(feeds) > 0:
                feed = feeds[0]
                timestamp = feed.get("created_at", "")
                temp = feed.get("field1", "")
                humi = feed.get("field2", "")
                gas = feed.get("field3", "")

                # Ghi vào CSV
                with open(FILENAME, mode='a', newline='', encoding='utf-8') as f:
                    writer = csv.writer(f)
                    writer.writerow([timestamp, temp, humi, gas])

                print(f"[{datetime.now().strftime('%H:%M:%S')}] Đã lưu: T={temp}°C, H={humi}%, MQ9={gas}ppm")
            else:
                print("Không có dữ liệu trả về.")
        else:
            print("lỗi tải dữ liệu:", response.status_code)

    except Exception as e:
        print("Lỗi:", e)

    time.sleep(INTERVAL)
