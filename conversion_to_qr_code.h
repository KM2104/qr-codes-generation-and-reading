#pragma once

#include <string>
#include <wx/bitmap.h>

wxBitmap GenerateQrCodeBitmap(const std::string& text, int scaleSize = 8, int border = 4);
