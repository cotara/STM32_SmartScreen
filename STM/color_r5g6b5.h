//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// ��� ����������� ����������� ���������������� ��������. �� ������ ���������
// ��� �� ����� �����, �� �� �������� ������� ������ �� ��� YouTube-����� 
// "����������� � ���������" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// �����: �������� ������ / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef _R5G6B5_H
#define _R5G6B5_H

//#include "../types.h"


// �������� ����� � ������� R5G6B5 ��� ������� ��������� ������
#define R5G6B5_RED      0xF800
#define R5G6B5_BLUE     0x001F
#define R5G6B5_GRAY     0x8430
#define R5G6B5_BLACK    0x0000
#define R5G6B5_WHITE    0xFFFF
#define R5G6B5_GREEN    0x07E0
#define R5G6B5_BROWN    0xBC40
#define R5G6B5_YELLOW   0xFFE0

typedef union
{
  struct
  {
    uint16_t B  :5;     // ����� ������������ ����� - ������� 5 ���
    uint16_t G  :6;     // ������ ������������ �����
    uint16_t R  :5;     // ������� ������������ - ������� 5 ���
  };
  uint16_t ColorValue;  // ��������, ���������� ���� � ���� 16-������� �����
}
tColor_R5G6B5;


#endif