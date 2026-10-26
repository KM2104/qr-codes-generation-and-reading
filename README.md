# QR codes generation and reading

Aplikacja desktopowa na Windows (C++17, `wxWidgets`) do:

- generowania kodów QR z tekstu,
- zapisu QR do `PNG` / `JPEG`,
- kopiowania QR do schowka jako obraz,
- przygotowania wiadomoœci e-mail z za³¹czonym QR,
- odczytu kodów QR z pliku obrazu,
- odczytu kodów QR z wycinka ekranu (schowek).

## Technologie

- C++17
- Visual Studio 2022 (`v143`)
- `wxWidgets` 3.2.11
- `Nayuki QR Code Generator` (generowanie)
- `ZXing-C++` (odczyt)
- `vcpkg` (zarz¹dzanie zale¿noœciami)

## Wymagania

- Windows x64
- Visual Studio 2022
- skonfigurowane `WXWIN` (œcie¿ka do instalacji `wxWidgets`)

## Zale¿noœci (`vcpkg`)

Projekt u¿ywa manifestu `vcpkg.json`.

Zainstaluj zale¿noœci w katalogu projektu:

```powershell
vcpkg install --triplet x64-windows-static-md
```

## Kompilacja

### Visual Studio

1. Otwórz plik `QR codes generation and reading.sln`.
2. Wybierz konfiguracjê `Release | x64`.
3. Uruchom `Build Solution`.

### MSBuild (CLI)

```powershell
msbuild ".\QR codes generation and reading.sln" /t:Build /p:Configuration=Release /p:Platform=x64
```

## Plik wykonywalny `.exe`

Po kompilacji w konfiguracji `Release|x64` plik programu znajduje siê w:

`x64\Release\QR codes generation and reading.exe`

## Struktura projektu

- `main.cpp` – punkt wejœcia aplikacji
- `MainFrame.h/.cpp` – GUI i logika akcji u¿ytkownika
- `conversion_to_qr_code.h/.cpp` – generowanie bitmapy QR
- `qrcodegen.hpp/.cpp` – biblioteka Nayuki
- `vcpkg.json` – manifest zale¿noœci

## Konfiguracje

- `Debug x64`
- `Release x64`

## Licencja

Projekt jest udostêpniany na licencji `MIT`.
Szczegó³y: plik `LICENSE`.

---

## Twórcy i Ÿród³a

- Autor projektu: `KM2104`
- Wspó³tworzenie kodu: GitHub Copilot

Repozytorium / pobranie programu:

- Kod Ÿród³owy: `https://github.com/KM2104/qr-codes-generation-and-reading`
- Wydania (`.exe`): `https://github.com/KM2104/qr-codes-generation-and-reading/releases`

> Jeœli nazwa repozytorium bêdzie inna, podmieñ tylko koñcówkê URL.


