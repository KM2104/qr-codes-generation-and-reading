#include <vector>
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <string>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/dcmemory.h>
#include <wx/colour.h>

#include "conversion_to_qr_code.h"

// Nag³ówek biblioteki Nayuki QR
#include "qrcodegen.hpp"

using qrcodegen::QrCode;

/**
 * Generuje wxBitmap zawieraj¹c¹ kod QR z podanego tekstu.
 *
 * @param text Tekst do zakodowania (np. URL).
 * @param scaleSize Szerokoœæ/wysokoœæ pojedynczego kwadracika (modu³u) w pikselach.
 * @param border Liczba pustych modu³ów jako margines wokó³ kodu (zalecane min. 4).
 * @return Gotowa do wyœwietlenia wxBitmap.
 */
wxBitmap GenerateQrCodeBitmap(const std::string& text, int scaleSize, int border)
{
    // 1. Wygeneruj kod QR przy u¿yciu biblioteki Nayuki (korekcja b³êdów: Medium)
    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::MEDIUM);

    // 2. Oblicz pe³ny rozmiar kodu w modu³ach (wraz z marginesem z obu stron)
    int qrSizeWithBorder = qr.getSize() + (border * 2);

    // 3. Oblicz ostateczny rozmiar bitmapy w pikselach ekranowych
    int pixelSize = qrSizeWithBorder * scaleSize;

    // 4. Utwórz czyst¹ bitmapê wxWidgets o odpowiednich wymiarach
    wxBitmap bitmap(pixelSize, pixelSize);

    // 5. U¿yj Memory DC (Device Context) do rysowania po bitmapie
    wxMemoryDC dc(bitmap);

    // Wype³nij ca³e t³o na bia³o
    dc.SetBackground(*wxWHITE_BRUSH);
    dc.Clear();

    // Ustaw pêdzel i pióro na czarne do rysowania modu³ów QR
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN); // Brak obramowania wokó³ kwadracików

    // 6. Rysuj piksele kodu QR
    // Przechodzimy pêtl¹ po ca³ej matrycy wygenerowanego kodu
    for (int y = 0; y < qr.getSize(); ++y)
    {
        for (int x = 0; x < qr.getSize(); ++x)
        {
            // SprawdŸ, czy dany punkt ma byæ czarny (true) czy bia³y (false)
            if (qr.getModule(x, y))
            {
                // Oblicz pozycjê na ekranie uwzglêdniaj¹c margines (border) i skalê
                int screenX = (x + border) * scaleSize;
                int screenY = (y + border) * scaleSize;

                // Narysuj czarny kwadracik o rozmiarze scaleSize x scaleSize
                dc.DrawRectangle(screenX, screenY, scaleSize, scaleSize);
            }
        }
    }

    // Od³¹cz bitmapê od kontekstu urz¹dzenia przed jej zwróceniem
    dc.SelectObject(wxNullBitmap);

    return bitmap;
}