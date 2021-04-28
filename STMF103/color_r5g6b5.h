//------------------------------------------------------------------------------
// This is Open source software. You can place this code on your site, but don't
// forget a link to my YouTube-channel: https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// Ёто программное обеспечение распростран€етс€ свободно. ¬ы можете размещать
// его на вашем сайте, но не забудьте указать ссылку на мой YouTube-канал 
// "Ёлектроника в объектике" https://www.youtube.com/channel/UChButpZaL5kUUl_zTyIDFkQ
// јвтор: Ќадыршин –услан / Nadyrshin Ruslan
//------------------------------------------------------------------------------
#ifndef _R5G6B5_H
#define _R5G6B5_H

//#include "../types.h"


// «начени€ цвета в формате R5G6B5 дл€ типовых сочетаний цветов
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
    uint16_t B  :5;     // —ин€€ составл€юща€ цвета - младшие 5 бит
    uint16_t G  :6;     // «елЄна€ составл€юща€ цвета
    uint16_t R  :5;     //  расна€ составл€юща€ - старшие 5 бит
  };
  uint16_t ColorValue;  // «начение, содержащее цвет в виде 16-битного слова
}
tColor_R5G6B5;


#endif