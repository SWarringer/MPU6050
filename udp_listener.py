import socket

# Configuration - must match your ESP32 settings
UDP_IP = "0.0.0.0"  # Listen on all network interfaces
UDP_PORT = 1234  # Must match UDP_SERVER_PORT in your udp.h


def main():
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # Bind to all interfaces on the specified port
    sock.bind((UDP_IP, UDP_PORT))

    print(f"Listening for UDP messages on port {UDP_PORT}...")
    print("Press Ctrl+C to stop\n")

    while True:
        try:
            # Receive data (1024 byte buffer)
            data, addr = sock.recvfrom(1024)

            # Decode and print the message
            message = data.decode("utf-8")
            print(f"Received from {addr[0]}:{addr[1]}: {message}")

            # Optional: Send a response back to ESP32
            response = f"OK: Received {len(data)} bytes"
            sock.sendto(response.encode(), addr)
            print(f"Sent response: {response}")

        except KeyboardInterrupt:
            print("\nShutting down...")
            break
        except Exception as e:
            print(f"Error: {e}")

    sock.close()


if __name__ == "__main__":
    main()
