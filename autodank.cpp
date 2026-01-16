#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

tesseract::TessBaseAPI g_ocr;

int xlock = 1300;
int ylock = 654;

POINT pixelshuntdig[] = {
    {702, 689},
    {752, 689},
    {802, 689}
};

int length_pixelshuntdig = sizeof(pixelshuntdig) / sizeof(pixelshuntdig[0]);
int result;

// ---- Press Enter function ----
void enter() {
    INPUT enter[2] = {0};
    enter[0].type = INPUT_KEYBOARD;
    enter[0].ki.wVk = VK_RETURN;

    enter[1].type = INPUT_KEYBOARD;
    enter[1].ki.wVk = VK_RETURN;
    enter[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, enter, sizeof(INPUT));
}

// ---- Cooldown in s function ----
void cooldown(int seconds) {
    Sleep(seconds * 1000);
}

// ---- Cooldown in ms function ----
void cooldown_ms(int ms) {
    Sleep(ms);
}

// ---- Instant Type function ----
void type(const char *msg) {
    int len = strlen(msg);

    // Worst-case: each char may need shift (up to 2 extra events per char)
    INPUT *inputs = (INPUT*)calloc(len * 4, sizeof(INPUT)); 
    int idx = 0;

    for (int i = 0; i < len; i++) {
        SHORT vk = VkKeyScanA(msg[i]);
        BYTE vkCode = vk & 0xFF;
        BYTE shift = (vk >> 8) & 0xFF;

        // Press SHIFT if needed
        if (shift & 1) {
            inputs[idx].type = INPUT_KEYBOARD;
            inputs[idx].ki.wVk = VK_SHIFT;
            inputs[idx].ki.dwFlags = 0; // key down
            idx++;
        }

        // Key down
        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = vkCode;
        inputs[idx].ki.dwFlags = 0;
        idx++;

        // Key up
        inputs[idx].type = INPUT_KEYBOARD;
        inputs[idx].ki.wVk = vkCode;
        inputs[idx].ki.dwFlags = KEYEVENTF_KEYUP;
        idx++;

        // Release SHIFT if needed
        if (shift & 1) {
            inputs[idx].type = INPUT_KEYBOARD;
            inputs[idx].ki.wVk = VK_SHIFT;
            inputs[idx].ki.dwFlags = KEYEVENTF_KEYUP;
            idx++;
        }
    }

    // Send all inputs at once
    SendInput(idx, inputs, sizeof(INPUT));
    free(inputs);
}

// ---- Type Integer function ----
void type_int(int number) {
    char buffer[16];
    sprintf(buffer, "%d", number);
    type(buffer);
}

// ---- Mouse Click function ----
void mouse_click() {
    INPUT input[2] = {0};

    // Mouse button down
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    // Mouse button up
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, input, sizeof(INPUT));
}

// ---- Detect Lockpad function ----
// radius = 1 → 3x3
// radius = 2 → 5x5
int is_yellow_near(int cx, int cy, int radius) {
    HDC hdc = GetDC(NULL);
    if (!hdc) return 0;

    for (int x = cx - radius; x <= cx + radius; x++) {
        for (int y = cy - radius; y <= cy + radius; y++) {

            COLORREF c = GetPixel(hdc, x, y);

            int r = GetRValue(c);
            int g = GetGValue(c);
            int b = GetBValue(c);

            // yellow-ish condition
            if (r > 150 &&
                g > 25 && g < 200 &&
                b < 100) {
                ReleaseDC(NULL, hdc);
                return 1;  // found yellow
            }
        }
    }

    ReleaseDC(NULL, hdc);
    return 0;  // not found
}

// ---- Number Detection Using OCR ----
HBITMAP capture_region(int x, int y, int w, int h) {
    HDC hScreen = GetDC(NULL);
    HDC hMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);

    SelectObject(hMem, hBitmap);
    BitBlt(hMem, 0, 0, w, h, hScreen, x, y, SRCCOPY);

    DeleteDC(hMem);
    ReleaseDC(NULL, hScreen);
    return hBitmap;
}

unsigned char* bitmap_to_buffer(HBITMAP hBitmap, int w, int h) {
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;   // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    unsigned char* buffer = (unsigned char*)malloc(w * h * 4);
    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hBitmap, 0, h, buffer, &bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    return buffer;
}

int ocr_number_from_region(int x1, int y1, int x2, int y2) {
    int w = x2 - x1;
    int h = y2 - y1;

    HBITMAP bmp = capture_region(x1, y1, w, h);
    unsigned char* img = bitmap_to_buffer(bmp, w, h);

    /*tesseract::TessBaseAPI api;
    if (api.Init(NULL, "eng")) {
        printf("Tesseract init failed\n");
        return -1;
    }

    // VERY IMPORTANT SETTINGS DONT DELETE
    api.SetPageSegMode(tesseract::PSM_SINGLE_WORD);
    api.SetVariable("tessedit_char_whitelist", "0123456789");

    api.SetImage(img, w, h, 4, w * 4);

    char* text = api.GetUTF8Text(); */   //orignal
	
	g_ocr.SetImage(img, w, h, 4, w * 4); //2nd fix
	char* text = g_ocr.GetUTF8Text();	 //2nd fix

    //int value = atoi(text);
	
	std::string digits;
	for (int i = 0; text[i]; i++) {
		if (text[i] >= '0' && text[i] <= '9') {
        digits += text[i];
		}
	}

	int value = digits.empty() ? -1 : atoi(digits.c_str());

    // cleanup
    delete[] text;
    //api.End();
    DeleteObject(bmp);
    free(img);

    return value;
}

//---- Commands List function ----
void cmd_beg() {
    type("pls beg");
    enter();
	cooldown(2);
	printf("Beg");
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
}

void cmd_fish() {
    type("pls fish catch");
    enter();
	cooldown(2);
	printf("Fish");
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
}

void cmd_hunt() {
    type("pls hunt");
    enter();
	cooldown(3);
	printf("Hunt");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		HDC hdc = GetDC(NULL);
		for(int i=0; i<length_pixelshuntdig; i++) {
			//SetCursorPos(pixelshuntdig[i].x,pixelshuntdig[i].y);
			COLORREF c = GetPixel(hdc, pixelshuntdig[i].x, pixelshuntdig[i].y);
			int r = GetRValue(c);
			int g = GetGValue(c);
			int b = GetBValue(c);
			
			if (r > 150 && g > 25 && g < 200 && b < 100) {
				if (i == 0 || i == 1) {
					SetCursorPos(861, 855);
					printf("Fireball at Left and Middle");
				}
				else if (i == 2){
					SetCursorPos(752, 855);
					printf("Fireball at Right");
				}
				mouse_click();
				break;
			}
		}
		ReleaseDC(NULL, hdc);
		cooldown(2);
	}
}

void cmd_dig() {
    type("pls dig");
    enter();
	cooldown(3);
	printf("Dig");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		HDC hdc = GetDC(NULL);
		for(int i=0; i<length_pixelshuntdig; i++) {
			//SetCursorPos(pixelshuntdig[i].x,pixelshuntdig[i].y);
			COLORREF c = GetPixel(hdc, pixelshuntdig[i].x, pixelshuntdig[i].y);
			int r = GetRValue(c);
			int g = GetGValue(c);
			int b = GetBValue(c);
					
			if ((r > 100 && r < 220 && g > 40 && g < 140 && b > 10 && b < 80) ||
			(r > 200 && g > 50 && g < 220 && b > 100 && b < 220)) {
				if (i == 0 || i == 1) {
					SetCursorPos(861, 855);
					printf("DigMonster at Left and Middle");
				}
				else if (i == 2){
					SetCursorPos(752, 855);
					printf("DigMonster at Right");
				}
				mouse_click();
				break;
			}
		}
		ReleaseDC(NULL, hdc);
		cooldown(2);
	}
}

void cmd_hl() {
    type("pls hl");
    enter();
	cooldown(3);
	printf("HL");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		int hlnum = ocr_number_from_region(989, 741, 1018, 765);
		//printf("%d\n", hlnum);
		
		if (hlnum <= 50) {
			SetCursorPos(887, 855);
		}
		else {
			SetCursorPos(632, 853);
		}
		mouse_click();
		cooldown(2);
	}
}

void cmd_search() {
    type("pls search");
    enter();
	cooldown(3);
	printf("Search");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		SetCursorPos(632, 853);
		mouse_click();
		cooldown(2);
	}
}

void cmd_tidy() {
    type("pls tidy");
    enter();
	cooldown(3);
	printf("Tidy");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		SetCursorPos(652, 843); //change x -> 792=broom 946=vacuum
		mouse_click();
		cooldown(2);
	}
}

void cmd_crime() {
    type("pls crime");
    enter();
	cooldown(3);
	printf("Crime");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		SetCursorPos(632, 853);
		mouse_click();
		cooldown(2);
	}
}

void cmd_postmemes() {
    type("pls postmemes");
    enter();
	cooldown(3);
	printf("Postmemes");
	
	if (is_yellow_near(xlock, ylock, 2)) {
			type("Theres a lockpad, cd 24s");
			enter();
			cooldown(24);
		}
	else {
		SetCursorPos(632, 853);
		mouse_click();
		cooldown(2);
	}
}

// ---- Scheduler Structure ----
typedef struct {
    DWORD last_run;
    DWORD cooldown_ms;
    void (*run)();
} Task;

// ---- Main function ----
int main() {
	if (g_ocr.Init(NULL, "eng")) {
		printf("OCR init failed\n");
		return 1;
	}

	g_ocr.SetPageSegMode(tesseract::PSM_SINGLE_WORD);
	g_ocr.SetVariable("tessedit_char_whitelist", "0123456789");

    SetProcessDPIAware();
    printf("Open Discord and focus chat...\n");
    cooldown(4);

    Task tasks[] = {
        {0, 41000, cmd_beg},      // 40s
        {0, 13000, cmd_fish},     // 12s
        {0, 21000, cmd_hunt},     // 20s
        {0, 21000, cmd_dig},      // 20s
        {0, 11000, cmd_hl},       // 10s
        {0, 26000, cmd_search},   // 25s
        {0, 41000, cmd_tidy},     // 40s
        {0, 41000, cmd_crime},    // 40s
        {0, 21000, cmd_postmemes},// 20s
    };

    int task_count = sizeof(tasks) / sizeof(tasks[0]);

    while (1) {
        DWORD now = GetTickCount();

        for (int i = 0; i < task_count; i++) {
            if (now - tasks[i].last_run >= tasks[i].cooldown_ms) {
                tasks[i].run();
                tasks[i].last_run = now;
                //Sleep(200); // human delay
            }
        }
        Sleep(50); // scheduler tick
    }
	g_ocr.End();
	return 0;
}