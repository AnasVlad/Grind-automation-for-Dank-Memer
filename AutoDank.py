import time
import pyautogui
import threading
import pytesseract
from PIL import ImageGrab

pytesseract.pytesseract.tesseract_cmd = r"C:\Program Files\Tesseract-OCR\tesseract.exe"

lock = threading.Lock()
total_runtime = 18000
end_time = time.time() + total_runtime

# your region coordinates for pls hl
x1, y1 = 991, 736
x2, y2 = 1023, 757

# pixel for dragon pls hunt
pixels = [(702, 689), (752, 689), (802, 689)]

time.sleep(4)

def hl_loop():
    while time.time() < end_time:
        with lock:
            pyautogui.typewrite('pls hl')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(3) #delay for buttons to show

            img = ImageGrab.grab(bbox=(x1, y1, x2, y2))
            text = pytesseract.image_to_string(img, config="--psm 8 -c tessedit_char_whitelist=0123456789")
            text = text.strip()

            if text.isdigit():
                num = int(text)
                print(num)
                if num <= 50:
                    pyautogui.moveTo(887, 855)
                else:
                    pyautogui.moveTo(632, 853)
            else:
               pyautogui.moveTo(632, 853)

            time.sleep(1)
            pyautogui.click()
            time.sleep(2)
        time.sleep(22) #cooldown

def other_loop():
    while time.time() < end_time:
        with lock:
            pyautogui.typewrite('pls fish catch')
            pyautogui.press('enter')
            time.sleep(2)
            """
            pyautogui.typewrite('pls dig')
            pyautogui.press('enter')
            time.sleep(2)
            """
            pyautogui.typewrite('pls hunt')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(3)

            for i, (x, y) in enumerate(pixels):
                r, g, b = pyautogui.pixel(x, y)
                if r > 150 and b < 100 and 25 < g < 200: #fire color condition
                    if i == 0 or i == 1:
                        pyautogui.moveTo(861, 855)
                    elif i == 2:
                        pyautogui.moveTo(752, 855)

                    time.sleep(1)
                    pyautogui.click()
                    print("Dragon")
                    break

            time.sleep(2)
        time.sleep(20) #cooldown

def buttons1_loop():
    while time.time() < end_time:
        with lock:
            pyautogui.typewrite('pls beg')
            pyautogui.press('enter')
            time.sleep(1)

            pyautogui.typewrite('pls tidy')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(3) #delay for buttons to show
            pyautogui.moveTo(652, 843) #792=broom 946=vacuum
            time.sleep(1)
            pyautogui.click()
            time.sleep(2)

            pyautogui.typewrite('pls crime')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(3) #delay for buttons to show
            pyautogui.moveTo(632, 853)
            time.sleep(1)
            pyautogui.click()
            time.sleep(2)
        time.sleep(40) #cooldown

def buttons2_loop():
    while time.time() < end_time:
        with lock:
            pyautogui.typewrite('pls postmemes')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(4) #delay for buttons to show
            pyautogui.moveTo(632, 853)
            time.sleep(1)
            pyautogui.click()
            time.sleep(2)
        time.sleep(30)

def buttons3_loop():
    while time.time() < end_time:
        with lock:
            pyautogui.typewrite('pls search')
            pyautogui.press('enter')
            #time.sleep(2)
            time.sleep(3) #delay for buttons to show
            pyautogui.moveTo(632, 853)
            time.sleep(1)
            pyautogui.click()
            time.sleep(2)
        time.sleep(27)

# start both loops as threads
t1 = threading.Thread(target=hl_loop)
t2 = threading.Thread(target=other_loop)
t3 = threading.Thread(target=buttons1_loop)
t4 = threading.Thread(target=buttons2_loop)
t5 = threading.Thread(target=buttons3_loop)

t1.start()
t2.start()
t3.start()
t4.start()
t5.start()

# optionally wait for both threads to finish
t1.join()
t2.join()
t3.join()
t4.join()
t5.join()
