import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime

FILENAME = "data.csv"

# Cấu hình matplotlib
plt.style.use('seaborn-v0_8-darkgrid')

# Tạo figure với 3 đồ thị: Nhiệt độ, Độ ẩm, MQ9
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8))
fig.suptitle("📊 Giám sát dữ liệu cảm biến realtime", fontsize=16, fontweight='bold')


def animate(i):
    try:
        # Đọc dữ liệu CSV
        df = pd.read_csv(FILENAME)

        # Chuyển thời gian sang định dạng datetime
        df['timestamp'] = pd.to_datetime(df['timestamp'])

        # Giữ tối đa 50 bản ghi gần nhất để dễ nhìn
        df = df.tail(50)

        # Xóa đồ thị cũ
        ax1.clear()
        ax2.clear()
        ax3.clear()

        # Nhiệt độ
        ax1.plot(df['timestamp'], df['temperature'], color='red', marker='o', label='Temperature (°C)')
        ax1.set_title("Temperature (°C)")
        ax1.set_ylabel("°C")

        # Độ ẩm
        ax2.plot(df['timestamp'], df['humidity'], color='blue', marker='o', label='Humidity (%)')
        ax2.set_title("Humidity (%)")
        ax2.set_ylabel("%")

        # MQ9 Gas
        ax3.plot(df['timestamp'], df['mq9'], color='green', marker='o', label='MQ9 (ppm)')
        ax3.set_title("Gas MQ9 (ppm)")
        ax3.set_ylabel("ppm")

        # Xoay nhãn trục thời gian cho dễ đọc
        for ax in (ax1, ax2, ax3):
            ax.tick_params(axis='x', rotation=30)
            ax.legend(loc='upper right')
            ax.grid(True)

        plt.tight_layout()
        plt.subplots_adjust(top=0.9)
    except Exception as e:
        print("Lỗi khi cập nhật biểu đồ:", e)


# Cập nhật mỗi 2 giây (đọc lại file data.csv)
ani = animation.FuncAnimation(fig, animate, interval=2000)

plt.show()
