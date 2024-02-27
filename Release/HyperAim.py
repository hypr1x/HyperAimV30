import ctypes
import cv2
import math
import numpy as np
import os
import sys
import time
import torch
from pystyle import Add, Center, Anime, Colors, Colorate, Write, System
import win32api
import win32gui
from termcolor import colored
import colorama
import win32con
import threading
import tkinter as tk
import dxcam
hwnd2 = win32gui.GetForegroundWindow()
os.system("title Hyper Aim")
aimbotEnabled = False
show = False
showAimbotStatus = True
resx = 1720
resy = 1080
settings = open("config", "r").read().split(" ")
fov = int(settings[0])
started = False
PUL = ctypes.POINTER(ctypes.c_ulong)
xy = 7.0
tar = 100.0
sens_config = {"xy_sens": xy, "targeting_sens": tar, "xy_scale": 10/xy, "targeting_scale": 1000/(tar * xy)}
model = torch.hub.load(
            "ultralytics/yolov5", "custom", path="best.pt", force_reload=True
        )
overlay = None
class KeyBdInput(ctypes.Structure):
    _fields_ = [
        ("wVk", ctypes.c_ushort),
        ("wScan", ctypes.c_ushort),
        ("dwFlags", ctypes.c_ulong),
        ("time", ctypes.c_ulong),
        ("dwExtraInfo", PUL),
    ]
class HardwareInput(ctypes.Structure):
    _fields_ = [
        ("uMsg", ctypes.c_ulong),
        ("wParamL", ctypes.c_short),
        ("wParamH", ctypes.c_ushort),
    ]
class MouseInput(ctypes.Structure):
    _fields_ = [
        ("dx", ctypes.c_long),
        ("dy", ctypes.c_long),
        ("mouseData", ctypes.c_ulong),
        ("dwFlags", ctypes.c_ulong),
        ("time", ctypes.c_ulong),
        ("dwExtraInfo", PUL),
    ]
class Input_I(ctypes.Union):
    _fields_ = [("ki", KeyBdInput), ("mi", MouseInput), ("hi", HardwareInput)]
class Input(ctypes.Structure):
    _fields_ = [("type", ctypes.c_ulong), ("ii", Input_I)]
class POINT(ctypes.Structure):
    _fields_ = [("x", ctypes.c_long), ("y", ctypes.c_long)]
class Aimbot:
    s = True
    extra = ctypes.c_ulong(0)
    ii_ = Input_I()
    screen = dxcam.create()
    aimbot_status = colored("ENABLED", "green")
    half_screen_width = ( ctypes.windll.user32.GetSystemMetrics(0) / 2 )  
    half_screen_height = ( ctypes.windll.user32.GetSystemMetrics(1) / 2) 
    def __init__(
        self, collect_data=False, mouse_delay=0, debug=False
    ):
        self.model = model
        self.model.iou = 0
        self.st = int(settings[1])/1000
        colorama.init()
        Aimbot.sens_config = sens_config
        self.pixel_increment = 0.7
        self.model.conf = int(settings[3])/100
        self.collect_data = collect_data
        self.mouse_delay = mouse_delay
        self.debug = debug
    def update_status_aimbot(self):
        global aimbotEnabled
        sys.stdout.write("\033[K")
        # if self.A: overlay.canvas.delete(self.A)
        if Aimbot.aimbot_status == colored("ENABLED", "green"):
            Aimbot.aimbot_status = colored("DISABLED", "red")
            # self.A = overlay.canvas.create_text(30, 180, text=(f"Aimbot: Disabled"), font=("Ariel", 9, "bold"), fill="#FFFFFF", justify="left", anchor="w")
            aimbotEnabled = False
            Write.Print(
            "    [!] AIMBOT IS DISABLED",
            Colors.blue_to_red,
            hide_cursor=True,
            interval=0.00,
            end="\r")
        else:
            Aimbot.aimbot_status = colored("ENABLED", "green")
            aimbotEnabled = True
            Write.Print(
            "    [!] AIMBOT IS ENABLED",
            Colors.blue_to_cyan,
            interval=0.00,
            hide_cursor=True,
            end="\r",)
    def left_click():
        ctypes.windll.user32.mouse_event(0x0002)  # left mouse down
        Aimbot.sleep(0.000001)
        ctypes.windll.user32.mouse_event(0x0004)  # left mouse up
    def sleep(duration, get_now=time.perf_counter):
        if duration == 0:
            return
        now = get_now()
        end = now + duration
        while now < end:
            now = get_now()
    def is_aimbot_enabled():
        return True if Aimbot.aimbot_status == colored("ENABLED", "green") else False
    def is_targeted():
        return True if win32api.GetKeyState(0x02) in (-127, -128) else False
    def move_crosshair(self, x, y):
        scale = Aimbot.sens_config["xy_scale"] * self.st

        for rel_x, rel_y in Aimbot.interpolate_coordinates_from_center(
            self, (x, y), scale
        ):
            Aimbot.ii_.mi = MouseInput(
                rel_x, rel_y, 0, 0x0001, 0, ctypes.pointer(Aimbot.extra)
            )
            input_obj = Input(ctypes.c_ulong(0), Aimbot.ii_)
            ctypes.windll.user32.SendInput(
                1, ctypes.byref(input_obj), ctypes.sizeof(input_obj)
            )
    def interpolate_coordinates_from_center(self, absolute_coordinates, scale):
        diff_x = (absolute_coordinates[0] - resx / 2) * scale / self.pixel_increment
        diff_y = (absolute_coordinates[1] - resy / 2) * scale / self.pixel_increment
        length = int(math.dist((0, 0), (diff_x, diff_y)))
        if length == 0:
            return
        unit_x = (diff_x / length) * self.pixel_increment
        unit_y = (diff_y / length) * self.pixel_increment
        x = y = sum_x = sum_y = 0
        for k in range(0, length):
            sum_x += x
            sum_y += y
            x, y = round(unit_x * k - sum_x), round(unit_y * k - sum_y)
            yield x, y
    def clean_up():
        Write.Print(
            "[INFO] F2 WAS PRESSED. QUITTING...", Colors.blue_to_purple, interval=0.000
        )
        os._exit(0)
    def p(self):
        print(f"\n    Discord - @yohyzr\n    Lock on Speed: {self.st:.20f}\n") 
        sys.stdout.write("\033[K")
        if Aimbot.aimbot_status == colored("ENABLED", "green"):
            Write.Print(
            "    [!] AIMBOT IS ENABLED",
            Colors.blue_to_cyan,
            interval=0.00,
            hide_cursor=True,
            end="\r")
        else:
            Write.Print(
            "    [!] AIMBOT IS DISABLED",
            Colors.blue_to_red,
            hide_cursor=True,
            interval=0.00,
            end="\r")
    def update(self):
        global overlay
        while True:
            try:
                settings = open("config", "r").read().split(" ")
                if int(settings[2]) == 1 and overlay:
                    overlay.circle_radius = int(settings[0]) / 2
                    overlay.hide()
                    overlay.show()
                if int(settings[2]) == 1 and not overlay:
                    overlay = Overlay()
                if int(settings[2]) == 0 and overlay:
                    overlay.callback()
                    overlay = None
                self.st = int(settings[1])/1000
                self.model.conf = int(settings[3])/100
                os.system("cls")
                self.p()
            except:pass
            time.sleep(2)
    def start(self):
        global aimbotEnabled
        self.sleep_d = 0
        self.p()
        global started
        global fov
        global hwnd2
        started = True
        threading.Thread(target=Aimbot.update, args=[self,]).start()
        self.update_status_aimbot()
        self.update_status_aimbot()
        detection_box = (int( Aimbot.half_screen_width - fov // 2 ), int(
                Aimbot.half_screen_height - fov // 2
                ),int(
                    Aimbot.half_screen_width + fov // 2
                ),int(
                    Aimbot.half_screen_height + fov // 2
                ))
        Aimbot.screen.start(detection_box, video_mode=True)
        while True: 
            # if not win32api.GetAsyncKeyState(0xA5) == 0 and started: 
            #     self.update_status_aimbot()
            #     Aimbot.sleep(0.1)
            frame = np.array(Aimbot.screen.get_latest_frame())
            results = self.model(frame)
            if len(results.xyxy[0]) != 0:  # player detected
                least_crosshair_dist = closest_detection = player_in_frame = False
                for *box, conf, cls in results.xyxy[
                    0
                ]:  # iterate over each player detected
                    x1y1 = [int(x.item()) for x in box[:2]]
                    x2y2 = [int(x.item()) for x in box[2:]]
                    x1, y1, x2, y2, conf = *x1y1, *x2y2, conf.item()
                    height = y2 - y1
                    relative_head_X, relative_head_Y = int((x1 + x2) / 2), int(
                        (y1 + y2) / 2 - height / 2.51
                    ) 
                    own_player = x1 < 15 or (
                        x1 < fov / 5 and y2 > fov / 1.2
                    ) 
                    crosshair_dist = math.dist(
                        (relative_head_X, relative_head_Y),
                        (fov / 2, fov / 2),
                    )

                    if not least_crosshair_dist:
                        least_crosshair_dist = crosshair_dist 
                    if crosshair_dist <= least_crosshair_dist and not own_player:
                        least_crosshair_dist = crosshair_dist
                        closest_detection = {
                            "x1y1": x1y1,
                            "x2y2": x2y2,
                            "relative_head_X": relative_head_X,
                            "relative_head_Y": relative_head_Y,
                            "conf": conf,
                        }
                    if own_player:
                        own_player = False
                if closest_detection:            
                    absolute_head_X, absolute_head_Y = (
                        closest_detection["relative_head_X"] + detection_box[0],
                        closest_detection["relative_head_Y"] + detection_box[1],
                    )
                    if Aimbot.is_aimbot_enabled() and Aimbot.is_targeted():
                        Aimbot.move_crosshair(self, absolute_head_X, absolute_head_Y)
            
            if cv2.waitKey(1) & 0xFF == ord("0"):
                break

os.system('cls')

class Overlay(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.start()

    def callback(self):
        self.root.after(0, self.root.destroy)

    def run(self):
        global hwnd
        global fov
        global hwnd3
        # Create the main window
        self.root = tk.Tk()

        # Set the window attributes to make it fully transparent
        self.root.config(bg="white")
        self.root.wm_attributes("-transparentcolor", "white")
        self.root.wm_attributes("-fullscreen", "True")
        self.root.wm_attributes("-topmost", "True")
        # Get screen dimensions
        screen_width = self.root.winfo_screenwidth()
        screen_height = self.root.winfo_screenheight()

        # Calculate the center of the screen
        self.center_x = screen_width // 2
        self.center_y = screen_height // 2

        # Create a canvas widget to draw the circle
        self.canvas = tk.Canvas(
            self.root,
            width=screen_width,
            height=screen_height,
            bg="#000000",
            highlightthickness=0,
        )
        hwnd = self.canvas.winfo_id()
        colorkey = win32api.RGB(0, 0, 0)  # full black in COLORREF structure
        wnd_exstyle = win32gui.GetWindowLong(hwnd, win32con.GWL_EXSTYLE)
        new_exstyle = wnd_exstyle | win32con.WS_EX_LAYERED
        win32gui.SetWindowLong(hwnd, win32con.GWL_EXSTYLE, new_exstyle)
        win32gui.SetLayeredWindowAttributes(hwnd, colorkey, 255, win32con.LWA_COLORKEY)
        self.canvas.pack()
        self.circle_radius = fov / 2 
        self.oval = self.canvas.create_oval(
            self.center_x - self.circle_radius,
            self.center_y - self.circle_radius,
            self.center_x + self.circle_radius,
            self.center_y + self.circle_radius,
            outline="#010101",
            width=2
        )
        hwnd = self.root.winfo_id()
        hwnd3 = self.canvas.winfo_id()
        # Start the Tkinter main loop
        self.root.mainloop()

    def hide(self):
        self.canvas.delete(self.oval)
        pass

    def show(self):
        self.oval = self.canvas.create_oval(
            self.center_x - self.circle_radius,
            self.center_y - self.circle_radius,
            self.center_x + self.circle_radius,
            self.center_y + self.circle_radius,
            outline="#010101",
            width=2
        )
        pass

hyper = Aimbot()
hyper.start()