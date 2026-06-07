#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_SSD1306 display(128, 64, &Wire, -1);

bool TerminalOpened = 1;
bool KeyboardOpened = 0;

String textInputBox = "";

bool cursor = 1;
bool cursorVisible = true;
unsigned long lastBlink = 0;

bool InputBarEnter = 1;
bool pressedInputBarEnter = 0;

const int KEY_COUNT = 51;
bool is_Caps = 0;
int kbrow = 0;
int kbcol = 0;

struct Key {
    const char* lower;
    const char* upper;    // заглавная

    int row;         // строка на клавиатуре
    int col;         // колонка

    int x;           // позиция x
    int y;           // позиция x

    int w;           // ширина кнопки
    int h;           // высота кнопки
};

const int rowLengths[] = {
    1,   
    14,  
    13, 
    12,  
    10,  
    1    
};

Key keyboard[KEY_COUNT] = {
    {"Enter", "Enter", 0, 0, 97, 3, 33, 12},
    {"`", "~", 1, 0, 6, 13, 9, 11},
    {"1", "!", 1, 1, 14, 13, 9, 11},
    {"2", "@", 1, 2, 22, 13, 9, 11},
    {"3", "#", 1, 3, 30, 13, 9, 11},
    {"4", "$", 1, 4, 38, 13, 9, 11},
    {"5", "%", 1, 5, 46, 13, 9, 11},
    {"6", "^", 1, 6, 54, 13, 9, 11},
    {"7", "&", 1, 7, 62, 13, 9, 11},
    {"8", "*", 1, 8, 70, 13, 9, 11},
    {"9", "(", 1, 9, 78, 13, 9, 11},
    {"0", ")", 1, 10, 86, 13, 9, 11},
    {"-", "_", 1, 11, 94, 13, 9, 11},
    {"=", "+", 1, 12, 102, 13, 9, 11},
    {"<-", "<-", 1, 13, 109, 13, 14, 11},
    {"q", "Q", 2, 0, 12, 23, 9, 12},
    {"w", "W", 2, 1, 20, 23, 9, 12},
    {"e", "E", 2, 2, 28, 23, 9, 12},
    {"r", "R", 2, 3, 36, 23, 9, 12},
    {"t", "T", 2, 4, 44, 23, 9, 12},
    {"y", "Y", 2, 5, 52, 23, 9, 12}, 
    {"u", "U", 2, 6, 60, 23, 9, 12},
    {"i", "I", 2, 7, 68, 23, 9, 12},
    {"o", "O", 2, 8, 76, 23, 9, 12},
    {"p", "P", 2, 9, 84, 23, 9, 12},
    {"[", "{", 2, 10, 92, 23, 9, 12},
    {"]", "}", 2, 11, 100, 23, 9, 12},
    {"\\", "|", 2, 12, 108, 23, 10, 12},
    {"Caps", "Caps", 3, 0, 14, 34, 12, 12},
    {"a", "A", 3, 1, 26, 34, 9, 12},
    {"s", "S", 3, 2, 34, 34, 9, 12},
    {"d", "D", 3, 3, 42, 34, 9, 12},
    {"f", "F", 3, 4, 50, 34, 9, 12},
    {"g", "G", 3, 5, 58, 34, 9, 12},
    {"h", "H", 3, 6, 66, 34, 9, 12},
    {"j", "J", 3, 7, 74, 34, 9, 12},
    {"k", "K", 3, 8, 82, 34, 9, 12},
    {"l", "L", 3, 9, 90, 34, 9, 12},
    {";", ":", 3, 10, 98, 34, 9, 12},
    {"'", "\"", 3, 11, 106, 34, 9, 12},
    {"z", "Z", 4, 0, 25, 45, 9, 12},
    {"x", "X", 4, 1, 33, 45, 9, 12},
    {"c", "C", 4, 2, 41, 45, 9, 12},
    {"v", "V", 4, 3, 49, 45, 9, 12},
    {"b", "B", 4, 4, 57, 45, 9, 12},
    {"n", "N", 4, 5, 65, 45, 9, 12},
    {"m", "M", 4, 6, 73, 45, 9, 12},
    {",", "<", 4, 7, 81, 45, 9, 12},
    {".", ">", 4, 8, 89, 45, 9, 12},
    {"/", "?", 4, 9, 97, 45, 9, 12},
    {"space", "space", 5, 0, 49, 55, 73, 10}
};

/*
   ___0____1_2_3_4_5_6_7_8_9__10_11____12
0 | Enter
1 |  `     2 3 4 5 6 7 8 9 0  -  =  Backspace
2 |   q     w e r t y u i o p [ ] \
3 |  Caps   a s d f g h j k l ; '
4 |   z     x c v b n m , . /
5 | space

*/

Key selectedKey = keyboard[0];

//ButtonChecker
bool btstat4 = 0;
bool btstat5 = 0;
bool btstat6 = 0;
bool btstat7 = 0;
bool btstat15 = 0;

bool btlaststat4 = 0;
bool btlaststat5 = 0;
bool btlaststat6 = 0;
bool btlaststat7 = 0;
bool btlaststat15 = 0;


const int MaxLines = 100;
String history[MaxLines];
int historySize = 0;
int scrollOffset = 0;

void drawEnterbutton() {
    if (InputBarEnter == 1) {
        display.drawRect(95, 52, 33, 12, SSD1306_WHITE);
        display.setCursor(97,55);
        display.print("Enter");
    }
    if (pressedInputBarEnter == 1) {
        display.fillRect(95, 52, 33, 12, SSD1306_WHITE);
        display.setTextColor(SSD1306_BLACK);
        display.setCursor(97,55);
        display.print("Enter");
        display.setTextColor(SSD1306_WHITE);
    }
}


void blinkedCursor() {
    if (millis() - lastBlink > 1000) {
    cursorVisible = !cursorVisible;
    lastBlink = millis();
    }

    if (cursorVisible)
    {
        display.print("_");
    }
}

void addResponce(String text) {

    while (text.length() > 21) {
        if (historySize < MaxLines) {
            history[historySize++] = text.substring(0, 21);
        }
        else {
            // сдвиг если переполнено
            for (int i = 1; i < MaxLines; i++) {
                history[i - 1] = history[i];
            }
            history[MaxLines - 1] = text.substring(0, 21);
        }

        text = text.substring(21);
    }

    if (historySize < MaxLines) {
        history[historySize++] = text;
    }
    else {
        for (int i = 1; i < MaxLines; i++) {
            history[i - 1] = history[i];
        }
        history[MaxLines - 1] = text;
    }

    if (historySize > 4) {
        scrollOffset = historySize - 4;
    }
}

void addLine(String text) {
    text = "> " + text;
    addResponce(text);
}

void addTextInputBox(String text) {
    textInputBox += text;
}

void clearTextInputBox() {
    textInputBox = "";
}

void drawTopBar() {
    display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setCursor(2, 3);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("Espnal V1.0");

    // display.setCursor(100, 2);
    // display.print(scrollOffset);
}

void drawInputBar() {
    if (TerminalOpened == 1) {
        drawEnterbutton();
    }

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    if (TerminalOpened == 1) {
        String input_text = "cmd> " + textInputBox;
        display.drawRect(0, 52, 128, 12, SSD1306_WHITE);
        display.setCursor(2, 55);
        display.print(input_text);
    }

    if (KeyboardOpened == 1) {
        String input_text = "> " + textInputBox;
        display.drawRect(0, 0, 128, 12, SSD1306_WHITE);
        display.setCursor(2, 3);
        display.print(input_text);
    }

    if (TerminalOpened && textInputBox != "") {
        cursor = 0;
    }
    else {
        cursor = 1;
    }


    if (cursor == 1) {
        blinkedCursor();
    }
} 

void drawTextArea() {
    for (int i = 0; i < 4; i++) {
        int lineIndex = scrollOffset + i;
        if (lineIndex < historySize) {
            display.setCursor(2, 14 + i * 10);
            display.print(history[lineIndex]);
        }
}
}

void drawTerminal() {
    display.clearDisplay();

    // ───── TOP BAR (контакт) ─────
    drawTopBar();
    // ───── CHAT AREA ─────
    drawTextArea();
    // ───── INPUT BOX (низ) ─────
    drawInputBar();

    display.display();
}


void drawKeyboard() {
    display.clearDisplay();

    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
    drawInputBar();

    for (int i = 0; i < KEY_COUNT; i++) {
        Key k = keyboard[i];
        bool selected = (k.row == kbrow && k.col == kbcol);
        if (selected) {
            display.setTextColor(SSD1306_BLACK);
            if (k.lower == "Enter") {
                display.fillRect(k.x - 2, k.y - 3, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "<-") {
                display.fillRect(k.x - 1, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "Caps") {
                display.fillRect(k.x - 1, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "space") {
                display.fillRect(k.x - 22, k.y - 1, k.w, k.h, SSD1306_WHITE);
            }
            else {
                display.fillRect(k.x - 2, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }
            display.setCursor(k.x, k.y);
            if (k.lower != "Caps") {
                if (is_Caps) {
                    display.print(k.upper);
                }
                else {
                    display.print(k.lower);
                }
            }
            else {
                if (!is_Caps) {
                    display.drawLine(15, 37, 18, 34, SSD1306_BLACK);
                    display.drawLine(19, 34, 22, 37, SSD1306_BLACK);
                    display.drawLine(16, 38, 17, 38, SSD1306_BLACK);
                    display.drawLine(20, 38, 21, 38, SSD1306_BLACK);
                    display.drawLine(17, 39, 17, 40, SSD1306_BLACK);
                    display.drawLine(20, 39, 20, 40, SSD1306_BLACK);
                    display.drawLine(18, 41, 19, 41, SSD1306_BLACK);
                }
                else {
                    display.drawLine(15, 37, 22, 37, SSD1306_BLACK);
                    display.drawLine(16, 36, 16, 38, SSD1306_BLACK);
                    display.drawLine(17, 35, 17, 40, SSD1306_BLACK);
                    display.drawLine(18, 34, 18, 41, SSD1306_BLACK);
                    display.drawLine(19, 34, 19, 41, SSD1306_BLACK);
                    display.drawLine(20, 35, 20, 40, SSD1306_BLACK);
                    display.drawLine(21, 36, 21, 38, SSD1306_BLACK);
                }
            }
        }
        else {
            display.setTextColor(SSD1306_WHITE);

            if (k.lower == "Enter") {
                display.drawRect(k.x - 2, k.y - 3, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "<-") {
                display.drawRect(k.x - 1, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "Caps") {
                display.drawRect(k.x - 1, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }
            else if (k.lower == "space") {
                display.drawRect(k.x - 22, k.y - 1, k.w, k.h, SSD1306_WHITE);
            }
            else {
                display.drawRect(k.x - 2, k.y - 2, k.w, k.h, SSD1306_WHITE);
            }

            display.setCursor(k.x, k.y);

            if (k.lower != "Caps") {
                if (is_Caps) {
                    display.print(k.upper);
                }
                else {
                    display.print(k.lower);
                }
            }
            else {
                if (!is_Caps) {
                    display.drawLine(15, 37, 18, 34, SSD1306_WHITE);
                    display.drawLine(19, 34, 22, 37, SSD1306_WHITE);
                    display.drawLine(16, 38, 17, 38, SSD1306_WHITE);
                    display.drawLine(20, 38, 21, 38, SSD1306_WHITE);
                    display.drawLine(17, 39, 17, 40, SSD1306_WHITE);
                    display.drawLine(20, 39, 20, 40, SSD1306_WHITE);
                    display.drawLine(18, 41, 19, 41, SSD1306_WHITE);
                }
                else {
                    display.drawLine(15, 37, 22, 37, SSD1306_WHITE);
                    display.drawLine(16, 36, 16, 38, SSD1306_WHITE);
                    display.drawLine(17, 35, 17, 40, SSD1306_WHITE);
                    display.drawLine(18, 34, 18, 41, SSD1306_WHITE);
                    display.drawLine(19, 34, 19, 41, SSD1306_WHITE);
                    display.drawLine(20, 35, 20, 40, SSD1306_WHITE);
                    display.drawLine(21, 36, 21, 38, SSD1306_WHITE);
                }
            }
        }

        
    }
    display.display();
}

Key getSelectedKey()
{
    for (int i = 0; i < KEY_COUNT; i++)
    {
        if (keyboard[i].row == kbrow && keyboard[i].col == kbcol)
        {
            return keyboard[i];
        }
    }

    return keyboard[0]; // fallback
}

void pressOK()
{
    Key k = getSelectedKey();

    if (k.lower == "Enter")
    {
        KeyboardOpened = 0;
        TerminalOpened = 1;
        return;
    }

    if (k.lower == "<-")
    {
        if (textInputBox.length() > 0)
            textInputBox.remove(textInputBox.length() - 1);
        return;
    }

    if (k.lower == "space")
    {
        textInputBox += " ";
        return;
    }

    if (k.lower == "Caps")
    {
        if (is_Caps) {
            is_Caps = 0;
        }
        else {
            is_Caps = 1;
        }
        return;
    }

    if (is_Caps)
        addTextInputBox(k.upper);
    else
        addTextInputBox(k.lower);
}

void clearHistory()
{
    historySize = 0;
    scrollOffset = 0;
}

void cleaningAnim() {
    for (int p = 0; p < 2; p++) {
        String first_string = "  |      Cleaning";
        for (int i = 0; i < 4; i++) {
            addResponce(first_string);
            addResponce("  |              .");
            addResponce(" /-\\       ,;\".:~");
            addResponce("//X\\\\  ..:.~;:\"");
            drawTerminal();
            first_string += ".";
            delay(500);
        }
    }
}


void cmdProcessing(String command) {
    addLine(command);
    if (command == "help") {
        addResponce("-help");
        addResponce("-clear");
        addResponce("-espnux");
        addResponce("-espfetch");
        addResponce("-author");
        addResponce("-?");
    } 
    else if (command == "espnux") {
        addResponce("espnux V1.0");
        addResponce("by @tootootoy");
        addResponce("(try GGMS)");
    }
    else if (command == "espfetch") {
        addResponce("Soon... :)");
    }
    else if (command == "clear") {
        cleaningAnim();
        clearHistory();
    }
    else if (command == "GGMS") {
        addResponce("God give me strength");
        addResponce("     _        _");
        addResponce("   (\\o/)    {\\o/}");
        addResponce("    /_\\      /_\\");
    }
    else if (command == "author") {
        addResponce("@tootootoy");
        addResponce("\"Never give up\"");
    }
    else if (command == "error404") {
        addResponce("wait...");
        // скример
    }
    else if (command == "wifi") {
        addResponce("Soon... :)");
        
    }
    else if (command == "ledON") {
        addResponce("Soon... :)");
    }
    else if (command == "ledOFF") {
        addResponce("Soon... :)");
    }
    else {
        String No_responce = "No such command \"" + command + "\"";
        addResponce(No_responce);
    }

}

void buttonChecker() {
    btstat4 = !digitalRead(4); // <-
    btstat5 = !digitalRead(5); // up
    btstat6 = !digitalRead(6); // down
    btstat7 = !digitalRead(7); // ->
    btstat15 = !digitalRead(15); // OK

    if (btstat4 == 1 && btlaststat4 == 0) {
        btlaststat4 = 1;
        if (kbcol > 0) {
            kbcol--;
        }
        else {
            kbcol = rowLengths[kbrow] - 1;
        }
    }
    if (btstat4 == 0 && btlaststat4 == 1) {
        btlaststat4 = 0;
    }
    if (btstat5 == 1 && btlaststat5 == 0) {
        btlaststat5 = 1;
        if (TerminalOpened) {
            if (scrollOffset > 0) {
                scrollOffset--;
            }
        }
        if (KeyboardOpened) {
            kbrow--;

            if (kbrow < 0) {
                kbrow = 0;
            }
            if (kbcol >= rowLengths[kbrow]) {
                kbcol = rowLengths[kbrow] - 1;
            }
        }
    }
    if (btstat5 == 0 && btlaststat5 == 1) {
        btlaststat5 = 0;
    }
    if (btstat6 == 1 && btlaststat6 == 0) {
        btlaststat6 = 1;
        if (TerminalOpened) {
            if (scrollOffset < historySize - 4) {
                scrollOffset++;
            }
        }
        if (KeyboardOpened) {
            kbrow++;

            if (kbrow > 5) {
               kbrow = 5;
            }

            if (kbcol >= rowLengths[kbrow]) {
                kbcol = rowLengths[kbrow] - 1;
            }
        }
    }
    if (btstat6 == 0 && btlaststat6 == 1) {
        btlaststat6 = 0;
    }
    if (btstat7 == 1 && btlaststat7 == 0) {
        btlaststat7 = 1;
        if (kbcol < rowLengths[kbrow] - 1) {
            kbcol++;
        }
        else {
            kbcol = 0;
        }
    }
    if (btstat7 == 0 && btlaststat7 == 1) {
        btlaststat7 = 0;
    }
    if (btstat15 == 1 && btlaststat15 == 0) {
        btlaststat15 = 1;

        if (TerminalOpened) {
            if (textInputBox == "") {
                TerminalOpened = 0;
                KeyboardOpened = 1;
            }
            else {
                cmdProcessing(textInputBox);
                textInputBox = "";
            }
            pressedInputBarEnter = 1;
        }
        else if (KeyboardOpened) {
            pressOK();
        }
        
    }
    if (btstat15 == 0 && btlaststat15 == 1) {
        btlaststat15 = 0;
        if (TerminalOpened) {
            pressedInputBarEnter = 0;
        }
    }
}



void setup() {
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
    pinMode(6, INPUT_PULLUP);
    pinMode(7, INPUT_PULLUP);
    pinMode(15, INPUT_PULLUP);
    
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.display();
}

void loop() {
    buttonChecker();
    if (TerminalOpened) {
        drawTerminal();
    }
    else if (KeyboardOpened) {
        drawKeyboard();
    }
}

//plans: wifi