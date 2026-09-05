import socket
import struct
import cv2
import mediapipe as mp
import numpy as np
import time


# ============================================================
# ESP32
# ============================================================

ESP32_IP = "10.91.219.221"

VIDEO_PORT = 81
CONTROL_PORT = 82


# ============================================================
# SETTINGS
# ============================================================

BUFFER_SIZE = 8192

MAX_FRAME_SIZE = 40000

WINDOW_NAME = "ESP32 OV3660 Gesture Control"


# ============================================================
# MEDIAPIPE
# ============================================================

mp_hands = mp.solutions.hands

mp_draw = mp.solutions.drawing_utils


hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,

    min_detection_confidence=0.6,

    min_tracking_confidence=0.6
)


# ============================================================
# RECEIVE EXACT BYTES
# ============================================================

def receive_exact(sock, size):

    data = bytearray()

    while len(data) < size:

        remaining = size - len(data)

        chunk = sock.recv(
            min(
                BUFFER_SIZE,
                remaining
            )
        )

        if not chunk:

            raise ConnectionError(
                "ESP32 disconnected"
            )

        data.extend(chunk)

    return bytes(data)


# ============================================================
# SEND LED COMMAND
# ============================================================

last_command = None


def send_led_command(command):

    global last_command

    # Don't send same command again
    if command == last_command:

        return

    try:

        control = socket.socket(
            socket.AF_INET,
            socket.SOCK_STREAM
        )

        control.settimeout(1.5)

        control.connect(
            (
                ESP32_IP,
                CONTROL_PORT
            )
        )

        message = (
            command + "\n"
        ).encode()

        control.sendall(
            message
        )

        # Wait for ESP32 acknowledgement
        try:

            control.recv(32)

        except socket.timeout:

            pass

        control.close()

        last_command = command

        print(
            "LED COMMAND:",
            command
        )

    except Exception as e:

        print(
            "LED control error:",
            e
        )


# ============================================================
# COUNT FINGERS
# ============================================================

def count_fingers(hand):

    landmarks = hand.landmark

    count = 0

    # --------------------------------------------------------
    # INDEX FINGER
    # --------------------------------------------------------

    if landmarks[8].y < landmarks[6].y:

        count += 1

    # --------------------------------------------------------
    # MIDDLE FINGER
    # --------------------------------------------------------

    if landmarks[12].y < landmarks[10].y:

        count += 1

    # --------------------------------------------------------
    # RING FINGER
    # --------------------------------------------------------

    if landmarks[16].y < landmarks[14].y:

        count += 1

    # --------------------------------------------------------
    # LITTLE FINGER
    # --------------------------------------------------------

    if landmarks[20].y < landmarks[18].y:

        count += 1

    return count


# ============================================================
# DETECT GESTURE
# ============================================================

def detect_gesture(fingers):

    if fingers == 4:

        return "OPEN PALM"

    elif fingers == 0:

        return "FIST"

    elif fingers == 1:

        return "ONE FINGER"

    elif fingers == 2:

        return "TWO FINGERS"

    else:

        return "UNKNOWN"


# ============================================================
# GESTURE → COMMAND
# ============================================================

def gesture_to_command(gesture):

    if gesture == "OPEN PALM":

        return "ON"

    elif gesture == "FIST":

        return "OFF"

    elif gesture == "ONE FINGER":

        return "SLOW"

    elif gesture == "TWO FINGERS":

        return "FAST"

    return None


# ============================================================
# CONNECT TO ESP32
# ============================================================

def connect_video():

    print(
        "Connecting to ESP32..."
    )

    sock = socket.socket(
        socket.AF_INET,
        socket.SOCK_STREAM
    )

    sock.setsockopt(
        socket.SOL_SOCKET,
        socket.SO_KEEPALIVE,
        1
    )

    sock.settimeout(15)

    sock.connect(
        (
            ESP32_IP,
            VIDEO_PORT
        )
    )

    print(
        "Connected successfully!"
    )

    return sock


# ============================================================
# MAIN
# ============================================================

def main():

    print()

    print(
        "=============================================="
    )

    print(
        "        ESP32 OV3660 GESTURE CONTROL"
    )

    print(
        "=============================================="
    )

    print()

    print(
        "ESP32 IP:"
    )

    print(
        ESP32_IP
    )

    print()

    print(
        "Video Port:"
    )

    print(
        VIDEO_PORT
    )

    print()

    print(
        "Control Port:"
    )

    print(
        CONTROL_PORT
    )

    print()

    print(
        "Gesture mapping:"
    )

    print(
        "OPEN PALM  = LED ON"
    )

    print(
        "FIST       = LED OFF"
    )

    print(
        "ONE FINGER = SLOW BLINK"
    )

    print(
        "TWO FINGERS= FAST BLINK"
    )

    # ========================================================
    # WINDOW
    # ========================================================

    cv2.namedWindow(
        WINDOW_NAME,
        cv2.WINDOW_NORMAL
    )

    cv2.resizeWindow(
        WINDOW_NAME,
        960,
        720
    )

    # ========================================================
    # CONNECT VIDEO
    # ========================================================

    try:

        sock = connect_video()

    except Exception as e:

        print()

        print(
            "Connection error:"
        )

        print(e)

        hands.close()

        cv2.destroyAllWindows()

        return

    # ========================================================
    # FPS
    # ========================================================

    frameCount = 0

    fps = 0.0

    fpsTime = time.time()

    # ========================================================
    # GESTURE
    # ========================================================

    currentGesture = "NO HAND"

    currentCommand = None

    try:

        print()

        print(
            "Receiving video..."
        )

        print(
            "Starting hand detection..."
        )

        print(
            "Press Q to quit."
        )

        print()

        while True:

            # =================================================
            # RECEIVE FRAME SIZE
            # =================================================

            header = receive_exact(
                sock,
                4
            )

            frameSize = struct.unpack(
                ">I",
                header
            )[0]

            # =================================================
            # VALIDATE FRAME SIZE
            # =================================================

            if frameSize <= 0:

                raise ValueError(
                    "Invalid frame size"
                )

            if frameSize > MAX_FRAME_SIZE:

                raise ValueError(
                    "Frame too large: "
                    + str(frameSize)
                )

            # =================================================
            # RECEIVE JPEG
            # =================================================

            jpeg = receive_exact(
                sock,
                frameSize
            )

            # =================================================
            # CHECK JPEG
            # =================================================

            if len(jpeg) < 4:

                continue

            # JPEG START

            if (
                jpeg[0] != 0xFF or
                jpeg[1] != 0xD8
            ):

                print(
                    "ERROR: Bad JPEG start"
                )

                continue

            # JPEG END

            if (
                jpeg[-2] != 0xFF or
                jpeg[-1] != 0xD9
            ):

                print(
                    "ERROR: Bad JPEG end"
                )

                continue

            # =================================================
            # DECODE
            # =================================================

            array = np.frombuffer(
                jpeg,
                dtype=np.uint8
            )

            frame = cv2.imdecode(
                array,
                cv2.IMREAD_COLOR
            )

            if frame is None:

                print(
                    "ERROR: OpenCV decode failed"
                )

                continue

            # =================================================
            # MIRROR
            # =================================================

            frame = cv2.flip(
                frame,
                1
            )

            # =================================================
            # MEDIAPIPE
            # =================================================

            rgb = cv2.cvtColor(
                frame,
                cv2.COLOR_BGR2RGB
            )

            result = hands.process(
                rgb
            )

            fingers = 0

            gesture = "NO HAND"

            command = None

            # =================================================
            # HAND FOUND
            # =================================================

            if result.multi_hand_landmarks:

                hand = (
                    result.multi_hand_landmarks[0]
                )

                # ---------------------------------------------
                # COUNT FINGERS
                # ---------------------------------------------

                fingers = count_fingers(
                    hand
                )

                # ---------------------------------------------
                # GESTURE
                # ---------------------------------------------

                gesture = detect_gesture(
                    fingers
                )

                # ---------------------------------------------
                # DRAW HAND
                # ---------------------------------------------

                mp_draw.draw_landmarks(
                    frame,
                    hand,
                    mp_hands.HAND_CONNECTIONS
                )

                # ---------------------------------------------
                # COMMAND
                # ---------------------------------------------

                command = gesture_to_command(
                    gesture
                )

                # ---------------------------------------------
                # SEND COMMAND
                # ---------------------------------------------

                if command is not None:

                    send_led_command(
                        command
                    )

                    currentCommand = command

            # =================================================
            # NO HAND
            # =================================================

            else:

                fingers = 0

                gesture = "NO HAND"

            currentGesture = gesture

            # =================================================
            # FPS
            # =================================================

            frameCount += 1

            elapsed = (
                time.time()
                - fpsTime
            )

            if elapsed >= 1.0:

                fps = (
                    frameCount /
                    elapsed
                )

                frameCount = 0

                fpsTime = time.time()

            # =================================================
            # RESOLUTION
            # =================================================

            height, width = (
                frame.shape[:2]
            )

            # =================================================
            # TEXT
            # =================================================

            cv2.putText(
                frame,
                f"OV3660  {width}x{height}",
                (20, 35),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8,
                (0, 255, 0),
                2
            )

            cv2.putText(
                frame,
                f"FPS: {fps:.1f}",
                (20, 70),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.8,
                (0, 255, 0),
                2
            )

            cv2.putText(
                frame,
                f"Gesture: {currentGesture}",
                (20, 110),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.75,
                (0, 255, 255),
                2
            )

            cv2.putText(
                frame,
                f"Fingers: {fingers}",
                (20, 145),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.7,
                (255, 255, 0),
                2
            )

            if currentCommand is not None:

                cv2.putText(
                    frame,
                    f"LED: {currentCommand}",
                    (20, 180),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.7,
                    (0, 255, 0),
                    2
                )

            cv2.putText(
                frame,
                "Q = Quit",
                (20, height - 20),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.6,
                (255, 255, 255),
                2
            )

            # =================================================
            # SHOW
            # =================================================

            cv2.imshow(
                WINDOW_NAME,
                frame
            )

            # =================================================
            # KEY
            # =================================================

            key = (
                cv2.waitKey(1)
                & 0xFF
            )

            if key == ord("q"):

                break

    except socket.timeout:

        print()

        print(
            "ERROR: Python socket timeout"
        )

    except ConnectionError as e:

        print()

        print(
            "Connection lost:"
        )

        print(e)

    except Exception as e:

        print()

        print(
            "Video/Gesture error:"
        )

        print(e)

    finally:

        # ----------------------------------------------------
        # TURN LED OFF
        # ----------------------------------------------------

        send_led_command(
            "OFF"
        )

        # ----------------------------------------------------
        # CLOSE VIDEO
        # ----------------------------------------------------

        sock.close()

        hands.close()

        cv2.destroyAllWindows()

        print()

        print(
            "Camera closed."
        )


# ============================================================
# START
# ============================================================

if __name__ == "__main__":

    main()
