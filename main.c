#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define genislik 100
#define yukseklik 28
#define DIKEY 179       // -
#define YATAY 196       // ¦
#define KOS_UL 218      // -
#define KOS_UR 191      // ¬
#define KOS_AL 192      // L
#define KOS_AR 217      // -
#define T_SOL   195   // +
#define T_SAG   180   // +
#define KESISIM 197   // +
#define T_ALT  194   // T  (yukarýdan gelen dikey + yatay)
#define T_UST  193   // +  (aþaðýdan gelen dikey + yatay)
#define HUD_YUKSEKLIK 2
#define ALT_YAZI_YUKSEKLIK 2
#define SUT_MESAFESI 8

typedef struct {
    int x, y;     // oyuncunun sahadaki konumu gösteriyor 
    int no;       // 1..8
    int takim;    // 0=kirmizi  1=siyah
} Oyuncu;

typedef struct {
    int x, y;
    int vx, vy;   
} Top;

Top top;
int mac_basladi = 0;

#define TAKIM_KIRMIZI 0
#define TAKIM_SIYAH   1
#define RENK_SAHA   0x2F   // Yeþil zemin 
#define RENK_KIRMIZI 0x4F  // Kýrmýzý
#define RENK_SIYAH   0x0F  // Beyaz

#define OYUNCU_SAYISI 16
Oyuncu oyuncular[OYUNCU_SAYISI];

int orta_dikdortgen_ust;
int orta_dikdortgen_alt;
int skor_kirmizi = 0;
int skor_siyah   = 0;
int yari = 1;                 // 1. yari / 2. yari
int mac_suresi_sn = 0;        // toplam saniye (0..2700)
int saha_ust;
int saha_alt;
int orta_y; 

int ceza_sol_x;
int ceza_sag_x;

DWORD last_tick = 0;
DWORD top_sahip_zamani = 0;
DWORD pas_zamani = 0;
DWORD sut_zamani = 0;
int top_sahibi = -1; 
int son_pas_veren = -1;
DWORD top_surmeye_baslama = 0;
char tahta[yukseklik][genislik];
int defans_kirmizi_max_x;
int defans_siyah_min_x;
int gol_bekleniyor = 0;
int son_golu_atan = -1;

int sut_kirmizi = 0, sut_siyah = 0;
int sut_isabet_kirmizi = 0, sut_isabet_siyah = 0;
int pas_kirmizi = 0, pas_siyah = 0;
int pas_basarili_kirmizi = 0, pas_basarili_siyah = 0;
int tehlikeli_atak_kirmizi = 0, tehlikeli_atak_siyah = 0;


char spiker_mesaj[120] = "Mac baslamaya hazir...";
DWORD spiker_zamani = 0;
const char* pas_yorumlari[] = {
    "Guzel pas!",
    "Akilli pas!",
    "Organize oyun!",
    "Top ayakta!",
    "Takim oyunu!"
};
const char* kapma_yorumlari[] = {
    "Mudahale basarili!",
    "Top kapildi!",
    "Araya girdi!",
    "Pozisyonu okudu!"
};
const char* sut_yorumlari[] = {
    "Kaleye sert bir vurus!",
    "Vurdu bakalim olacak mi?",
    "Sut cekildi!",
    "Tehlikeli deneme!"
};
const char* rastgele(const char** dizi, int adet) {
    return dizi[rand() % adet];
}

void temizle_tahta(void);
void ciz_tahta(void);
void ciz_saha_cercevesi(void);
void ciz_orta_cizgi(void);
void ciz_ceza_sahasi_sol(void);
void ciz_ceza_sahasi_sag(void);
void ciz_kale_sol(void);
void ciz_kale_sag(void);
void ciz_orta_dikdortgen(void);
void oyunculari_baslat(void);
void oyunculari_ciz(void);
void yaz_yazi(int x, int y, const char *s);
int en_yakin_takim_arkadasi(int);
void pas_at(void);
void topu_sur(void);
void imlec_gizle(void);
void topu_baslat(void);
void topu_ciz(void);
void topu_hareket_ettir(void);
void oyunculari_hareket_ettir(void);
void gol_kontrol_ve_santra(void);
void santra_yap(void);
void spiker_yaz(const char *s);
const char* rastgele(const char** dizi, int adet);

int yakin_oyuncu_var_mi(int nx, int ny, int ignore_idx) {
	int j;
    for (j = 0; j < OYUNCU_SAYISI; j++) {
        if (j == ignore_idx) continue;

        if (abs(oyuncular[j].x - nx) <= 1 && abs(oyuncular[j].y - ny) <= 1)
            return 1;
    }
    return 0;
}

int main() {
	system("chcp 437 > nul"); 
    system("color 2F");
    
    imlec_gizle();   
    
    saha_ust = HUD_YUKSEKLIK;
    saha_alt = yukseklik - ALT_YAZI_YUKSEKLIK - 1;
    orta_y   = (saha_ust + saha_alt) / 2;
    
    ceza_sol_x = 20;
    ceza_sag_x = genislik - 21;
    
    topu_baslat();
    ciz_orta_dikdortgen();
    oyunculari_baslat();
    srand(GetTickCount());

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)  //Esc ile çýkýþ
		break;

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {     // space ile kontrol
            if (gol_bekleniyor) {
            gol_bekleniyor = 0;      // Gol sonrasý ortadan baþlasýn 
            santra_yap();
            mac_basladi = 1;
            last_tick = GetTickCount();
            Sleep(200);
            }
            // === DEVRE ARASI / MAC BASLANGICI ===
            else if (!mac_basladi) {

            // 2. yari ise ortadan basla
               if (yari == 2) {
                  santra_yap();
            }

           mac_basladi = 1;
           last_tick = GetTickCount();
           Sleep(200);
           }
        }
        if (mac_basladi) {                         // süre güncelle
            DWORD now = GetTickCount();
            if (now - last_tick >= 1000) {          // 1 saniye geçti
                mac_suresi_sn++;
                last_tick += 1000;
                // 45 dk = 2700 sn (istersen burada 45*60 yap)
                if (mac_suresi_sn == 2*60) yari = 2;
                if (mac_suresi_sn >= 4*60) mac_basladi = 0; // þimdilik durdur
                if (mac_suresi_sn == 2*60) {
                    yari = 2;
                    mac_basladi = 0;    // <<< DUR, SPACE BEKLE
                    spiker_yaz("ILK YARI BITTI! 2. YARI ICIN SPACE");
                }
                if (mac_suresi_sn == 4*60) {
                    mac_basladi = 0;
                    
                    temizle_tahta();
                    
                    yaz_yazi(genislik/2 - 6, 4, "MAC SONUCU");
                    yaz_yazi(genislik/2 - 18, 5, "----------------------------------");
                    
                    char satir[80];

                    sprintf(satir, "KIRMIZI   %d  -  %d   SIYAH", skor_kirmizi, skor_siyah);
                    yaz_yazi(genislik/2 - 15, 7, satir);

                    sprintf(satir, "Sut (Isabetli): %d(%d)  -  %d(%d)", sut_kirmizi, sut_isabet_kirmizi,sut_siyah, sut_isabet_siyah);
                    yaz_yazi(genislik/2 - 15, 9, satir);
                    sprintf(satir, "Pas Basarisi: %d%%  -  %d%%", pas_kirmizi ? (pas_basarili_kirmizi*100)/pas_kirmizi : 0, pas_siyah ? (pas_basarili_siyah*100)/pas_siyah : 0); 
					yaz_yazi(genislik/2 - 15, 11, satir);
                    sprintf(satir, "Tehlikeli Atak: %d  -  %d",tehlikeli_atak_kirmizi, tehlikeli_atak_siyah); 
					yaz_yazi(genislik/2 - 15, 13, satir);

                    yaz_yazi(genislik/2 - 6, yukseklik/2+5, "CIKIS ICIN ESC");

                   ciz_tahta();

                   while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
                          Sleep(50);
                   }
                   exit(0);
                }

            }
        }
        
        temizle_tahta();

        if (mac_basladi) {
           oyunculari_hareket_ettir();
          
           if (top_sahibi == -1) {
           topu_hareket_ettir();            // herkes topa koþar   // top serbest gider
           }
           else {
            topu_sur();   // <<< TOP OYUNCUNUN ÖNÜNDE DURUR
                if (GetTickCount() - top_sahip_zamani > 200) {      // pas süresi dolduysa
        	       if (rand() % 100 < 60)
                   pas_at();
                   else
                   top_sahip_zamani = GetTickCount();
                }
            }
        }
        ciz_saha_cercevesi();
        ciz_ceza_sahasi_sol();
        ciz_ceza_sahasi_sag();
        ciz_orta_dikdortgen();
        ciz_kale_sol();
        ciz_kale_sag();
        ciz_orta_cizgi(); 
    
        oyunculari_ciz();
        topu_ciz();   // sadece sahipsizken çiz
        defans_kirmizi_max_x = genislik / 2 - 8;   // orta çizgiye yaklaþamaz
        defans_siyah_min_x  = genislik / 2 + 8;
 
        int dakika = (mac_suresi_sn % (2*60)) / 60;          // ÜST HUD (SKOR / YARI / SAAT)
        int saniye = (mac_suresi_sn % (2*60)) % 60;
        
        char hud[128];
        
        sprintf(hud, "SKOR: [ KRM %d - %d SYH ]   %d. YARI  %02d:%02d   (ESC: Cikis)",
                skor_kirmizi, skor_siyah, yari, dakika, saniye);
                
        yaz_yazi(2, 0, hud);
    
        if (gol_bekleniyor) {                         // GOL EKRANI 
           yaz_yazi(genislik/2 - 14, yukseklik/2 - 1, "G O O L!  (Devam icin SPACE)");
           if (son_golu_atan == TAKIM_KIRMIZI)
               yaz_yazi(2, yukseklik-1, "SPOR SERVISI: Gol! KIRMIZI TAKIM atti!");
           else
              yaz_yazi(2, yukseklik-1, "SPOR SERVISI: Gol! SIYAH TAKIM atti!");
        }

        if (!mac_basladi&&!gol_bekleniyor) {                 // ORTA “Baslamak icin SPACE”
            if (yari == 1)
                yaz_yazi(genislik/2 - 10, (yukseklik/2)-1, "Baslamak icin SPACE");
            else
               yaz_yazi(genislik/2 - 9, (yukseklik/2)-1, "2. YARI ICIN SPACE");
            yaz_yazi(2, yukseklik-1, "SPOR SERVISI: MAC BASLAMAYA HAZIR...");
        }
        
        if (GetTickCount() - spiker_zamani < 3000) {          // ===== SPOR SERVISI YAZISI =====
        char alt[160];
        sprintf(alt, "SPOR SERVISI: %s", spiker_mesaj);
        yaz_yazi(2, yukseklik - 1, alt);
        }
        
        ciz_tahta();
        Sleep(30); // akýcý görünüm
    }
	return 0;
}

void temizle_tahta(void) {
	int r,c;
    for (r = 0; r < yukseklik; r++)
    for (c = 0; c < genislik; c++)
    tahta[r][c] = ' ';
}

void ciz_tahta(void) {
    
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(h, pos);
    
    int r, c;
    for (r = 0; r < yukseklik; r++) {
        for (c = 0; c < genislik; c++) {
            char ch = tahta[r][c];
            int renk = RENK_SAHA;
            
            if (ch >= '1' && ch <= '8') {     // Oyuncu mu?
                int i;                              // Hangi takým?
                for (i = 0; i < OYUNCU_SAYISI; i++) {
                if (oyuncular[i].x == c && oyuncular[i].y == r) {
                        if (oyuncular[i].takim == TAKIM_KIRMIZI)
                            renk = RENK_KIRMIZI;
                        else
                            renk = RENK_SIYAH;
                        break;
                    }
                }
            }
            SetConsoleTextAttribute(h, renk);
            putchar(ch);
        }
		putchar('\n');
    }  		
		
    SetConsoleTextAttribute(h, RENK_SAHA);
}

void ciz_saha_cercevesi(void) {
    int r,c;
    tahta[saha_ust][0] = KOS_UL;           // köþeler
    tahta[saha_ust][genislik-1] = KOS_UR;
    tahta[saha_alt][0] = KOS_AL;
    tahta[saha_alt][genislik-1] = KOS_AR;

    for (c = 1; c < genislik-1; c++) {
        tahta[saha_ust][c] = YATAY;
        tahta[saha_alt][c] = YATAY;
    }
    for (r =saha_ust+1 ; r < saha_alt; r++) {
        tahta[r][0] = 	DIKEY;
        tahta[r][genislik-1] = DIKEY;
    }
    int cx = genislik / 2;
}

void ciz_orta_cizgi(void) {
    int x = genislik / 2;
    int r;
    
    for (r = saha_ust; r < saha_alt; r++) {
	  if (r == orta_dikdortgen_ust || r == orta_dikdortgen_alt)
	    tahta[r][x] = KESISIM;
	  else
	    tahta[r][x] = DIKEY;
    }
 
    tahta[saha_ust][x] = T_ALT;   // T         // Üst saha kenarý birleþimi
    tahta[saha_alt][x] = T_UST; // +         // Alt saha kenarý birleþimi
}

void ciz_orta_dikdortgen() {
    int gen = 12; // geniþlik
    int yuk = 8;  // yükseklik
    int orta_y = (saha_ust + saha_alt) / 2;
    orta_y += 1;
    int orta_x = genislik / 2;
    int ust = orta_y - yuk/2;       
    int alt = orta_y + yuk/2;       
    int sol = orta_x - gen / 2;
    int sag = orta_x + gen / 2;

    orta_dikdortgen_ust = ust;
    orta_dikdortgen_alt = alt;

    int c,r;
    
    for (c = sol+1; c <= sag; c++){
	
        tahta[ust][c] = YATAY;
        tahta[alt][c] = YATAY; 
	}

    for (r = ust+1; r <= alt; r++){
        tahta[r][sol] = DIKEY;
        tahta[r][sag] = DIKEY;
	}
	
    tahta[ust][sol] = KOS_UL;
    tahta[ust][sag] = KOS_UR;
    tahta[alt][sol] = KOS_AL;
    tahta[alt][sag] = KOS_AR;
    
    int cx = genislik / 2;

    tahta[ust][cx] = KESISIM;           // orta çizgi – dikdörtgen üst
    tahta[alt][cx] = KESISIM;          // orta çizgi – dikdörtgen alt
}

void ciz_ceza_sahasi_sol(void) {
	int ceza_yuk = 16;
    int ust = orta_y - ceza_yuk / 2;  
    int alt = orta_y + ceza_yuk / 2;
    int sol = 0;
    int sag = 20;
    int r, c;
    
    for (c =1; c <= sag; c++){             // üst çizgi
        tahta[ust][c] = YATAY;
        tahta[alt][c] = YATAY;
    }
    
    for (r = ust; r <= alt; r++){          // sað dikey çizgi
        tahta[r][sag] = DIKEY;
        tahta[r][sol] = DIKEY;
    }
  
    tahta[ust][sag] = KOS_UR;   // ¬            // tahta[ust][sol] = KOS_UL;   // -
    tahta[alt][sag] = KOS_AR;   // -            // tahta[alt][sol] = KOS_AL;   // L
    
    tahta[ust][sol] = T_SOL;   // üst birleþim         // saha kenarý ile birleþimler
    tahta[alt][sol] = T_SOL;  // alt birleþim
}

void ciz_ceza_sahasi_sag(void) {
    int ceza_yuk = 16;                
    int ust = orta_y - ceza_yuk / 2;  
    int alt = orta_y + ceza_yuk / 2;
    int sag = genislik - 1;
    int sol = genislik - 21;
    int r, c;

    for (c = sol; c <= sag; c++)
        tahta[ust][c] = YATAY;
        
    for (c = sol; c <= sag; c++)
        tahta[alt][c] = YATAY;

    for (r = ust; r <= alt; r++)
        tahta[r][sol] = DIKEY;
        
    for (r = ust; r <= alt; r++)
        tahta[r][sag] = DIKEY;   
        
    tahta[ust][sol] = KOS_UL;
    tahta[ust][sag] = KOS_UR;
    tahta[alt][sol] = KOS_AL;
    tahta[alt][sag] = KOS_AR; 
        
    tahta[ust][genislik-1] = DIKEY;
    tahta[alt][genislik-1] = DIKEY;
   
    tahta[ust][sag] = T_SAG;   // üst birleþim             // saha kenarý ile birleþimler
    tahta[alt][sag] = T_SAG;  // alt birleþim
}

void ciz_kale_sol(void) {
    int orta = orta_y;
    int ust = orta - 3;
    int alt = orta + 3;
    int r;
   
    tahta[ust][0] = KOS_UL;                // Üst çýkýntý
    tahta[ust][1] = YATAY;
    tahta[ust][2] = YATAY;
    
    for (r = orta_y-2; r <= orta_y+2; r++)      // Dikey direk
    tahta[r][0] = DIKEY;
    
    tahta[alt][0] = KOS_AL;               // Alt çýkýntý
    tahta[alt][1] = YATAY;
    tahta[alt][2] = YATAY;
}

void ciz_kale_sag(void) {
    int orta = orta_y;
    int ust = orta - 3;
    int alt = orta + 3;
    int r;
    int x = genislik - 1;
    
    tahta[ust][x] = KOS_UR;              // Üst çýkýntý sola doðru
    tahta[ust][x - 1] = YATAY;
    tahta[ust][x - 2] = YATAY;

    for (r =orta_y-2; r < orta_y+2; r++)         // Dikey direk
        tahta[r][x] = DIKEY;

    tahta[alt][x] = KOS_AR;              // Alt çýkýntý sola doðru
    tahta[alt][x - 1] = YATAY;
    tahta[alt][x - 2] = YATAY;
}

void oyunculari_baslat(void) {

    int i = 0;
    int orta_x = genislik / 2;
    
    //kýrmýzý takým
    oyuncular[i++] = (Oyuncu){3, orta_y, 1, TAKIM_KIRMIZI};            // 1 - Kaleci

    oyuncular[i++] = (Oyuncu){ceza_sol_x, orta_y - 6, 2, TAKIM_KIRMIZI};      // 2 - 3 Defans (ceza sahasý çizgileri)
    oyuncular[i++] = (Oyuncu){ceza_sol_x, orta_y + 6, 3, TAKIM_KIRMIZI};

    oyuncular[i++] = (Oyuncu){ceza_sol_x + 10, orta_y, 4, TAKIM_KIRMIZI};        // 4 - Orta saha (ceza sahasý DIÞI)

    oyuncular[i++] = (Oyuncu){ceza_sol_x + 19, orta_y - 7, 5, TAKIM_KIRMIZI};       // 5 - 6 Orta saha
    oyuncular[i++] = (Oyuncu){ceza_sol_x + 19, orta_y + 7, 6, TAKIM_KIRMIZI};

    oyuncular[i++] = (Oyuncu){orta_x - 3, orta_dikdortgen_ust, 7, TAKIM_KIRMIZI};       // 7 - 8 Forvet (orta dikdörtgen kenarlarý)
    oyuncular[i++] = (Oyuncu){orta_x - 3, orta_dikdortgen_alt, 8, TAKIM_KIRMIZI};

    //siyah takim
    oyuncular[i++] = (Oyuncu){genislik - 4, orta_y, 1, TAKIM_SIYAH};            // 1 - Kaleci

    oyuncular[i++] = (Oyuncu){ceza_sag_x, orta_y - 6, 2, TAKIM_SIYAH};           // 2 - 3 Defans
    oyuncular[i++] = (Oyuncu){ceza_sag_x, orta_y + 6, 3, TAKIM_SIYAH};

    oyuncular[i++] = (Oyuncu){ceza_sag_x - 10, orta_y, 4, TAKIM_SIYAH};           // 4 - Orta saha (ceza sahasý DIÞI)

    oyuncular[i++] = (Oyuncu){ceza_sag_x - 19, orta_y - 7, 5, TAKIM_SIYAH};        // 5 - 6 Orta saha
    oyuncular[i++] = (Oyuncu){ceza_sag_x - 19, orta_y + 7, 6, TAKIM_SIYAH};

    oyuncular[i++] = (Oyuncu){orta_x + 3, orta_dikdortgen_ust, 7, TAKIM_SIYAH};         // 7 - 8 Forvet
    oyuncular[i++] = (Oyuncu){orta_x + 3, orta_dikdortgen_alt, 8, TAKIM_SIYAH};
}

void oyunculari_ciz(void) {
    int i;
    for (i = 0; i < OYUNCU_SAYISI; i++) {
        int x = oyuncular[i].x;
        int y = oyuncular[i].y;

        if (x <= 0 || x >= genislik-1) continue;
        if (y <= 0 || y >= yukseklik-1) continue;

        if (oyuncular[i].takim == TAKIM_KIRMIZI)
            tahta[y][x] = '0' + oyuncular[i].no;   // kýrmýzý numara
        else
            tahta[y][x] = '0' + oyuncular[i].no;   // siyah numara
    }
}

void topu_baslat(void) {
    top.x = genislik / 2;
    top.y = orta_y;
    top.vx = 0;
    top.vy = 0;
}

void topu_ciz(void) {
    tahta[top.y][top.x] = 'o';   // YUVARLAK TOP
}

void yaz_yazi(int x, int y, const char *s) {
    int i = 0;
    while (s[i] != '\0') {
        int xx = x + i;
        if (y >= 0 && y < yukseklik && xx >= 0 && xx < genislik) {
            tahta[y][xx] = s[i];
        }
        i++;
    }
}

void imlec_gizle(void) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(h, &info);
}

int hucrede_oyuncu_var_mi(int x, int y) {
    int i;
    for (i = 0; i < OYUNCU_SAYISI; i++) {
        if (oyuncular[i].x == x && oyuncular[i].y == y)
        return 1;
    }
    return 0;
}
 
void topu_hareket_ettir(void) {
	sut_zamani = 0;
	if (!mac_basladi) return;
	if (top_sahibi != -1) return;   // <<< TOP OYUNCUDAYSA ASLA BURAYA GÝRME
	
    static DWORD son_hareket = 0;
    DWORD simdi = GetTickCount();

    if (simdi - son_hareket < 80) return;            // 80 ms = yavaþ ve temiz hareket
    son_hareket = simdi;

    int nx = top.x + top.vx;             // === 1) YENI KONUMU ONCE HESAPLA ===
    int ny = top.y + top.vy;

    if ((nx <= 1 || nx >= genislik - 2) &&                // ===== GOL KONTROLÜ (DUVARA ÇARPMADAN ÖNCE) =====
    ny >= orta_y - 2 && ny <= orta_y + 2) {

       top.x = nx;
       top.y = ny;
       gol_kontrol_ve_santra();
       if (gol_bekleniyor) return;
    }
    
    int i; 
   
    for (i = 0; i < OYUNCU_SAYISI; i++) {                //sýkýþma anýnda topu kurtarýyor
        if (oyuncular[i].x == nx && oyuncular[i].y == ny) {

            if (rand() % 100 < 60) {                   // %60 ihtimalle topu alsýn
                top_sahibi = i;
                
                if (son_pas_veren != -1 &&                           // basarili pas (topu alan kisi ayni takimdansa) istatistik için
                   oyuncular[son_pas_veren].takim == oyuncular[i].takim) {

                if (oyuncular[i].takim == TAKIM_KIRMIZI) pas_basarili_kirmizi++;
                    else pas_basarili_siyah++;
                }
                top_sahip_zamani = GetTickCount();
                top.vx = 0;
                top.vy = 0;
            return;
            }

        
            top.vx = (rand() % 3) - 1;            // top sekerek kurtulmasý için
            top.vy = (rand() % 3) - 1;

            if (top.vx == 0 && top.vy == 0)
                top.vx = (rand() % 2) ? 1 : -1;
            return;
        }
    }

    if (nx<= 1) {           // Sol duvar
    nx= 2;
    top.vx = abs(top.vx);   // saga dogru
    }
    else if (nx >= genislik - 2) {       // Sag duvar
    nx = genislik - 3;
    top.vx = -abs(top.vx); // sola dogru
    }

    if (ny <= saha_ust + 1) {    // Ust duvar
    ny = saha_ust + 2;
    top.vy = abs(top.vy);  // asagi
    }


    else if (ny >= saha_alt - 1) {       // Alt duvar
    ny = saha_alt - 2;
    top.vy = -abs(top.vy); // yukari
    }
    
    top.x = nx;
    top.y = ny;
    gol_kontrol_ve_santra();
    
    // yan çizgi sýkýþma önleme
    if (top.x <= 2 || top.x >= genislik - 3) {
       top.vy = (rand() % 3) - 1;
    }

}

void oyunculari_hareket_ettir(void) {

    static DWORD son_adim = 0;
    DWORD simdi = GetTickCount();
    if (simdi - son_adim < 120) return;
    son_adim = simdi;
    int i;
    for (i = 0; i < OYUNCU_SAYISI; i++) {

        Oyuncu *o = &oyuncular[i];
        int dx = 0, dy = 0;
        
        if (o->no == 1) {                 // KALECÝ SABÝT DAVRANIÞ
            int hedef_y = top.y;
            if (hedef_y < orta_y - 2) hedef_y = orta_y - 2;
            if (hedef_y > orta_y + 2) hedef_y = orta_y + 2;

            int ny = o->y;
            if (ny < hedef_y) ny++;
            else if (ny > hedef_y) ny--;
            if (!yakin_oyuncu_var_mi(o->x, ny, i))
                o->y = ny;
            continue;
        }

        if (i == top_sahibi) continue;            // top sahibi zaten topu_sur() ile hareket ediyor
        if (top_sahibi == -1) {                   //TOP KÝMDE?
            // TOP BOÞTA: herkes (kendi alaný uygunsa) topa yönelsin
            if (o->x < top.x) dx = 1;
            else if (o->x > top.x) dx = -1;

            if (o->y < top.y) dy = 1;
            else if (o->y > top.y) dy = -1;
        }
        else {
            int top_takim = oyuncular[top_sahibi].takim;     // TOP BÝRÝNDE
            if (o->takim == top_takim) {
            	// ===== ORTA SAHA FORVET DESTEGI (5-6) =====
            if ((o->no == 5 || o->no == 6) &&
               (oyuncular[top_sahibi].no == 7 || oyuncular[top_sahibi].no == 8)) {
               dx = (o->takim == TAKIM_KIRMIZI) ? 1 : -1;

               if (o->y < oyuncular[top_sahibi].y) dy = 1;
               else if (o->y > oyuncular[top_sahibi].y) dy = -1;
               else dy = 0;

               
               if (o->takim == TAKIM_KIRMIZI && o->x > ceza_sol_x - 2) dx = 0;            // ceza sahasi sinirinda dur
               if (o->takim == TAKIM_SIYAH   && o->x < ceza_sag_x + 2) dx = 0;
            }
			 
	        else {
        
                  dx = (o->takim == TAKIM_KIRMIZI) ? 1 : -1;          // ===== AYNI TAKIM: PAS ÝÇÝN AÇIL =====
                  int d = abs(o->x - oyuncular[top_sahibi].x) + abs(o->y - oyuncular[top_sahibi].y);
                  if (d < 4) {
                    dy = (rand() % 2) ? 1 : -1;
                    dx = 0;
                   } 
				  else {
                    int r = rand() % 100;
                    if (r < 33) dy = -1;
                    else if (r < 66) dy = 1;
                    else dy = 0;
                  }    
            }	
            }
        else {
            int d = abs(o->x - top.x) + abs(o->y - top.y);       // ===== RAKÝP: BASKI YAP =====
            if (d < 12) { // yakýnsa topa koþ
                if (o->x < top.x) dx = 1;
                else if (o->x > top.x) dx = -1;
                if (o->y < top.y) dy = 1;
                else if (o->y > top.y) dy = -1;
                }
            }
        }

        // ===== ALAN SINIRI 
        int alan_sol, alan_sag;
        if (o->takim == TAKIM_KIRMIZI) {
            if (o->no <= 3) { alan_sol = 0; alan_sag = genislik / 2 - 5; }
            else { alan_sol = genislik / 4; alan_sag = genislik - 1; }
        } else {
            if (o->no <= 3) { alan_sol = genislik / 2 + 5; alan_sag = genislik - 1; }
            else { alan_sol = 0; alan_sag = genislik * 3 / 4; }
        }
        
        // top boþta deðilse alan dýþýna uçmasýnlar diye sýnýr uygula
        int nx = o->x + dx;
        int ny = o->y + dy;
        
        // ===== KALE ICINE GIRME ENGELI =====
       int kale_ust = orta_y - 2;
       int kale_alt = orta_y + 2;

       // sol kale (x = 0,1,2 civarý)
       if (o->takim == TAKIM_KIRMIZI) {
       if (nx <= 2 && ny >= kale_ust && ny <= kale_alt) {
           nx = o->x;   // geri al
           ny = o->y;
        }
    }

// sag kale (x = genislik-3,genislik-2)
if (o->takim == TAKIM_SIYAH) {
    if (nx >= genislik - 3 && ny >= kale_ust && ny <= kale_alt) {
        nx = o->x;
        ny = o->y;
    }
}
        // ===== CEZA SAHASI SINIRI (FORVET DISI GIREMEZ) =====
       if (o->no <= 4) { // SADECE GK + DEF + 4 NUMARA
          if (o->takim == TAKIM_KIRMIZI) {
              if (nx > ceza_sol_x)
              nx = ceza_sol_x; 
           }  
		  else {
                if (nx < ceza_sag_x)
                nx = ceza_sag_x;
           } 
        }
        // ===== DEFANS CIZGISI KORUMA =====
        if (o->no <= 3) { // defans oyuncularý
            if (o->takim == TAKIM_KIRMIZI) {
               if (nx > defans_kirmizi_max_x)
               nx = defans_kirmizi_max_x;
            } 
            else { // SIYAH
            if (nx < defans_siyah_min_x)
            nx = defans_siyah_min_x;
            }
        }

        if (nx < alan_sol) nx = alan_sol;
        if (nx > alan_sag) nx = alan_sag;
        
        if (nx <= 0 || nx >= genislik - 1) continue;        // saha sýnýrý
        if (ny <= saha_ust || ny >= saha_alt) continue;

        if (yakin_oyuncu_var_mi(nx, ny, i)) continue;        // minimum mesafe
        
        if (o->takim == TAKIM_KIRMIZI && o->no <= 3) {       // ===== KIRMIZI DEFANS KÝLÝTLENME KURTARMA =====
            if (nx == o->x) {              // ileri gidemiyorsa Y ekseninde kay
                if (rand() % 100 < 30) {
                   int ty = o->y + ((rand() % 2) ? 1 : -1);
                   if (ty > saha_ust && ty < saha_alt && !yakin_oyuncu_var_mi(o->x, ty, i)) {
                       ny = ty;
                    }
                }
            }
        }

        o->x = nx;
        o->y = ny;

       
        if (top_sahibi == -1 && abs(o->x - top.x) <= 1 && abs(o->y - top.y) <= 1) {	     // top boþta ise top kapma
           char msg[120];
           sprintf(msg, "%s-%s%d %s",
                  o->takim == TAKIM_KIRMIZI ? "KRM" : "SYH",
                  (o->no <= 3) ? "DEF" : (o->no <= 6) ? "MID" : "FWD",
                  o->no,
                  rastgele(kapma_yorumlari, 4)
           );
           spiker_yaz(msg);
	
           top_sahibi = i;
           top_sahip_zamani = GetTickCount();
           top.vx = 0;
           top.vy = 0;
        }
    }
}

int en_yakin_takim_arkadasi(int idx) {
    int i;
    int best = -1;
    int bestd = 999999;

    for (i = 0; i < OYUNCU_SAYISI; i++) {
        if (i == idx) continue;
        if (oyuncular[i].takim != oyuncular[idx].takim) continue;

        int dx = oyuncular[i].x - oyuncular[idx].x;
        int dy = oyuncular[i].y - oyuncular[idx].y;
        int d2 = dx*dx + dy*dy;

        if (d2 < bestd) { bestd = d2; best = i; }
    }
    return best;
}

void pas_at(void) {
	
    if (top_sahibi == -1) return;

    int hedef = en_yakin_takim_arkadasi(top_sahibi);
    if (hedef == -1) return;

    int sx = oyuncular[top_sahibi].x;
    int sy = oyuncular[top_sahibi].y;
    int tx = oyuncular[hedef].x;
    int ty = oyuncular[hedef].y;

    son_pas_veren = top_sahibi;              // topu serbest býrak
    
    if (oyuncular[top_sahibi].takim == TAKIM_KIRMIZI)  pas_kirmizi++;    
	else pas_siyah++;
    

    char msg[120];
    sprintf(msg, "%s-%s%d %s",
            oyuncular[son_pas_veren].takim == TAKIM_KIRMIZI ? "KRM" : "SYH",
            (oyuncular[son_pas_veren].no <= 3) ? "DEF" :
            (oyuncular[son_pas_veren].no <= 6) ? "MID" : "FWD",
            oyuncular[son_pas_veren].no,
            rastgele(pas_yorumlari, 5)
    );
    spiker_yaz(msg);

    top_sahibi = -1;
    pas_zamani = GetTickCount();

    top.vx = (tx > sx) ? 1 : (tx < sx ? -1 : 0);              // yönü -1/0/1 yap
    top.vy = (ty > sy) ? 1 : (ty < sy ? -1 : 0);

    if (top.vx == 0 && top.vy == 0)                // tamamen ayný hizada kalmasýn diye küçük bir garanti
        top.vx=1;
        
    top.x = sx + top.vx;
    top.y = sy + top.vy;   
    
    if (top.x <= 1) top.x = 1;
    if (top.x >= genislik - 2) top.x = genislik - 2;
    if (top.y <= saha_ust + 1) top.y = saha_ust + 1;
    if (top.y >= saha_alt - 1) top.y = saha_alt - 1;
        
}

void topu_sur(void) {
	int kale_ust = orta_y - 2;
    int kale_alt = orta_y + 2;

    if (sut_zamani != 0 && GetTickCount() - sut_zamani < 200)            // ÞUT SONRASI KISA DONMA
    return;
    if (top_sahibi == -1) return;
    if (oyuncular[top_sahibi].no == 1) return;           // kaleci sürmez
    Oyuncu *o = &oyuncular[top_sahibi];
    
    if (o->no == 5 || o->no == 6) {
    if (rand() % 100 < 3) {
        char msg[100];
        sprintf(msg, "%s-MID%d Top akiyor!",
            o->takim == TAKIM_KIRMIZI ? "KRM" : "SYH",
            o->no
        );
        spiker_yaz(msg);
        }
    }

    int kale_x = (o->takim == TAKIM_KIRMIZI) ? genislik - 2 : 1;             // === KALEYE YAKINSA SUT ===

    int kale_mesafe = abs(o->x - kale_x);

    // ===== FORVETLER ZORUNLU SUT =====
    if ((o->no == 7 || o->no == 8) && kale_mesafe <= 8 && o->y >= kale_ust && o->y <= kale_alt) {
    	// ISTATISTIK: sut
        if (o->takim == TAKIM_KIRMIZI) sut_kirmizi++;
        else sut_siyah++;

       top_sahibi = -1;
       pas_zamani = GetTickCount();

       top.vx = (o->takim == TAKIM_KIRMIZI) ? 2 : -2;
       top.vy = (rand() % 3) - 1;
       if (top.vy == 0) top.vy = 1;
          top.x = o->x + top.vx;
       top.y = o->y;
   
       char msg[100];
       sprintf(msg, "%s-FWD%d Kaleye sert bir sut!",o->takim == TAKIM_KIRMIZI ? "KRM" : "SYH", o->no);
       spiker_yaz(msg);
    return;
    }

    if (abs(o->x - kale_x) <= SUT_MESAFESI && o->y >= kale_ust && o->y <= kale_alt) {
        if (rand() % 100 < 70) {
        	// ISTATISTIK: sut
           if (o->takim == TAKIM_KIRMIZI) sut_kirmizi++;
           else sut_siyah++;

           top_sahibi = -1;                    // === TOP OYUNCUDAN AYRILIR ===
           pas_zamani = GetTickCount();

           top.vx = (o->takim == TAKIM_KIRMIZI) ? 2 : -2;           // top hýzlanýr
           top.vy = (rand() % 3) - 1;
           if (top.vy == 0) top.vy = 1;
           top.x = o->x+top.vx;              // ?? TOPU OYUNCUNUN ÜZERÝNE KOYMA!
           top.y = o->y;
           
           // === OYUNCU ÞUT ANINDA DURUR ===
           // bu frame’de baþka hareket yapma
           sut_zamani = GetTickCount();
           char msg[120];
           sprintf(msg, "%s-FWD%d %s", o->takim == TAKIM_KIRMIZI ? "KRM" : "SYH", o->no, rastgele(sut_yorumlari, 4));
           spiker_yaz(msg);
        return;
        }
    }

    static DWORD son_adim = 0;
    DWORD simdi = GetTickCount();
    if (simdi - son_adim < 140) return;
    son_adim = simdi;

    int dx = 0, dy = 0;
    
    dx = (o->takim == TAKIM_KIRMIZI) ? 1 : -1;          // ileri yön
    
    if (o->no >= 7) {
        if (o->y < orta_y) dy = 1;
        else if (o->y > orta_y) dy = -1;
    }

    int karar = rand() % 100;             // ==== YAN KAÇMA KARARI ====

    if (karar < 25) {
        dy = -1;        // yukarý kaç
    }
    else if (karar < 50) {
        dy = 1;         // aþaðý kaç
    }
    else {
        dy = 0;         // düz devam
    }

    int nx = o->x + dx;
    int ny = o->y + dy;
    
    if (nx <= 1 || nx >= genislik - 2) return;               // saha sýnýrý
    if (ny <= saha_ust || ny >= saha_alt) ny = o->y;

    int i;                                        // oyuncuya çarpma engeli
    for (i = 0; i < OYUNCU_SAYISI; i++) {
        if (i != top_sahibi && oyuncular[i].x == nx && oyuncular[i].y == ny) {
            return;
        }
    }
    
    o->x = nx;                         // === HAREKET ===
    o->y = ny;

    top.x = o->x + dx;                    // topu önüne koy
    top.y = o->y;

    if (top.y <= saha_ust)                    // üst sýnýr düzeltme
        top.y = o->y + 1;
    if (top.y >= saha_alt) 
	    top.y = o->y - 1;
        
    if (abs(o->x - kale_x) > 10) {
        if (rand() % 100 < 30)
            pas_at();
    }
     

}

void spiker_yaz(const char *s) {
    strncpy(spiker_mesaj, s, sizeof(spiker_mesaj)-1);
    spiker_mesaj[sizeof(spiker_mesaj)-1] = '\0';
    spiker_zamani = GetTickCount();   // <<< ÇOK ÖNEMLÝ
}


void gol_kontrol_ve_santra(void) {

    int kale_ust = orta_y - 2;
    int kale_alt = orta_y + 2;

    // === SOL KALE (SIYAH GOL) ===
    if (top.x <=1 && top.y >= kale_ust && top.y <= kale_alt) {
        skor_siyah++;
        sut_isabet_siyah++;
        son_golu_atan = TAKIM_SIYAH;
        
        mac_basladi = 0;
        gol_bekleniyor = 1;

        spiker_yaz("GOOOL! SIYAH TAKIM!");
        
        top.vx = 0;            // topu durdur
        top.vy = 0;
        return;
    }

    // === SAG KALE (KIRMIZI GOL) ===
    if (top.x >= genislik - 2 && top.y >= kale_ust && top.y <= kale_alt) {

        skor_kirmizi++;
        sut_isabet_kirmizi++;
        spiker_yaz("Gol oluyor!");
        son_golu_atan = TAKIM_KIRMIZI;
        
        mac_basladi = 0;
        gol_bekleniyor = 1;

        spiker_yaz("GOOOL! KIRMIZI TAKIM!");

        top.vx = 0;             // topu durdur
        top.vy = 0;
        return;
    }
}

void santra_yap(void) {
    top.x = genislik / 2;
    top.y = orta_y;
    top.vx = 0;
    top.vy = 0;

    top_sahibi = -1;
    pas_zamani = GetTickCount();

    oyunculari_baslat();
    
    top.vx = (rand() % 2 == 0) ? 1 : -1;
    top.vy = (rand() % 3) - 1;
    if (top.vx == 0 && top.vy == 0) top.vx = 1;
}

