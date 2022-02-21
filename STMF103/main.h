#ifndef MAIN_H
#define MAIN_H 

#define HOURBUFSIZE 600
#define ANOTHERBUFSIZE 60
//Температруа снаружи
#define HOURT1STARTPAGE 0
#define HOURT1ENDTPAGE 9
#define DAYT1STARTPAGE 10
#define DAYT1ENDTPAGE 19
#define WEEKT1STARTPAGE 20
#define WEEKT1ENDTPAGE 29
#define MONTHT1STARTPAGE 30
#define MONTHT1ENDTPAGE 39
//Температруа внутри
#define HOURT2STARTPAGE 40
#define HOURT2ENDTPAGE 49
#define DAYT2STARTPAGE 50
#define DAYT2ENDTPAGE 59
#define WEEKT2STARTPAGE 60
#define WEEKT2ENDTPAGE 69
#define MONTHT2STARTPAGE 70
#define MONTHT2ENDTPAGE 79
// Влажность
#define HOURHSTARTPAGE 80
#define HOURHENDTPAGE 89
#define DAYHSTARTPAGE 90
#define DAYHENDTPAGE 99
#define WEEKHSTARTPAGE 100
#define WEEKHENDTPAGE 109
#define MONTHHSTARTPAGE 110
#define MONTHHENDTPAGE 119
//Давление
#define HOURPSTARTPAGE 120
#define HOURPENDTPAGE 129
#define DAYPSTARTPAGE 130
#define DAYPENDTPAGE 139
#define WEEKPSTARTPAGE 140
#define WEEKPENDTPAGE 149
#define MONTHPSTARTPAGE 150
#define MONTHPENDTPAGE 159

void updateEEPROM();
void sensorsDataSend();
//Карта первой страницы EEPROM

//0 - текущая страница записи дня (7-11)
//1 - текущая страница записи недели (12-16)
//2 - текущая страница записи месяца (17-21)
#endif /* MAIN_H */