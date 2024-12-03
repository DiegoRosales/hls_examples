# %%
import sys
import numpy as np
from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget
import pyqtgraph as pg
from PyQt6.QtCore import QTimer
import pickle
import socket
import time


# %%
def get_spectrum_from_socket(conn, listening_time):
    # Receive data
    start_time = time.time()
    while time.time() - start_time < listening_time:
        try:
            # Receive data
            data = conn.recv(4096)
            if not data:
                break

            # Verify the length of the received data
            expected_length = 2199  # Adjust based on your data size
            if len(data) != expected_length:
                print(f"Received incomplete data {len(data)}. Waiting for more.")
                continue

            # Deserialize the received data into an array
            fft_results = pickle.loads(data)

            return np.abs(np.fft.fftshift(fft_results))

        except socket.timeout:
            # Handle timeout (no data received within the timeout period)
            print("Timeout occurred. No data received within the timeout period.")
            continue


# %%
class RealTimePlotWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("Frequency spectrum")
        self.setGeometry(100, 100, 800, 600)

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)

        self.layout = QVBoxLayout()
        self.central_widget.setLayout(self.layout)

        self.plot_widget = pg.PlotWidget()
        self.layout.addWidget(self.plot_widget)

        self.plot_widget.setBackground("k")  # Set black background for plot area
        self.plot_widget.setTitle("Frequency Spectrum", color="w", size="20pt")
        self.plot_widget.setLabel("left", "Amplitude")
        self.plot_widget.setLabel("bottom", "Frequency [kHz]")

        N = 256
        fs = 48e3 / 4
        self.x_data = (
            np.fft.fftshift(np.fft.fftfreq(N, d=1 / fs)) / 1e3
        )  # Create frequency axis
        self.y_data = []

        self.curve = self.plot_widget.plot(pen="y")

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start(50)  # Update plot every 50 milliseconds

        # Create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Bind the socket to a port
        receiver_port = 12345
        s.bind(("0.0.0.0", receiver_port))

        # Listen for incoming connections
        s.listen(1)

        # Accept a connection
        self.conn, addr = s.accept()

        self.listening_time = 60

    def update_plot(self):
        # Generate random data
        self.y_data = get_spectrum_from_socket(self.conn, self.listening_time)

        # Plot data
        if self.y_data is not None:
            self.curve.setData(self.x_data, self.y_data)
        else:
            self.conn.close()
            self.stop_and_close()

    def stop_and_close(self):
        self.timer.stop()  # Stop the QTimer
        self.close()  # Close the window


# %%
app = QApplication(sys.argv)
window = RealTimePlotWindow()
window.show()
sys.exit(app.exec())
