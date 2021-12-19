void delay(unsigned int count);
void Write_Command(unsigned char command);
void Write_Data(unsigned char data1);
void SendData(unsigned int color);
void Init_SSD1963(void);
void WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y);
void SetPixel(unsigned int x,unsigned int y, unsigned int color);
void FillColor(unsigned int color);
