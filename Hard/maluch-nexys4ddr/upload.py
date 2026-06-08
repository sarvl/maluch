import serial
import sys
import time

SERIAL_PORT = '/dev/ttyUSB1'
BAUD_RATE = 115200

def upload_bin(file_path):
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Opened {SERIAL_PORT} at {BAUD_RATE} baud.")
        
        with open(file_path, 'rb') as f:
            data = f.read()
            
        print(f"Sending {len(data)} bytes ({len(data)//2} words)...")
        
        # Send data all at once. The hardware can keep up with 115200 baud.
        ser.write(data)
        ser.flush()
        time.sleep(0.5)
            
        print("Upload complete!")
        ser.close()
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python upload.py <file.bin>")
    else:
        upload_bin(sys.argv[1])
