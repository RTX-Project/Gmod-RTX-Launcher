// Theme.h - тёмная тема оформления в духе GOG Galaxy 2.0:
// сайдбар слева, плоские скруглённые кнопки с ховер/press-состояниями,
// акцентный цвет, тёмные панели. Рисуется через GDI+
// (входит в стандартный Windows SDK, доп. установка не нужна).
#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <map>
#include <algorithm>
#include <mutex>
#include "resource.h"

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")

using namespace Gdiplus;

namespace Theme {

    inline const Color kBgMain      (255, 13, 15, 18);   // глубокий тёмный тёмно-угольный (#0D0F12)
    inline const Color kBgSidebar   (255, 9, 10, 12);    // сайдбар
    inline const Color kBgPanel     (255, 22, 25, 32);   // фон карточек (Glassmorphism dark)
    inline const Color kBgInput     (255, 28, 32, 40);   // фон полей ввода
    inline const Color kBgConsole   (255, 8, 9, 11);     // фон консоли

    // Изумрудно-зеленый неоновый акцент (#00E676)
    inline const Color kAccentMetro      (255, 0, 230, 118);
    inline const Color kAccentMetroHover (255, 30, 245, 135);
    inline const Color kAccentMetroPress (255, 0, 195, 100);

    // Бирюзово-голубой RTX акцент (#00B0FF)
    inline const Color kAccentRtx      (255, 0, 176, 255);
    inline const Color kAccentRtxHover (255, 64, 196, 255);
    inline const Color kAccentRtxPress (255, 0, 145, 230);

    inline float g_accentTransition = 0.0f; // 0.0 = Metro, 1.0 = RTX

    inline Color LerpColor(const Color& c1, const Color& c2, float t) {
        BYTE a = (BYTE)(c1.GetA() + (float)(c2.GetA() - c1.GetA()) * t);
        BYTE r = (BYTE)(c1.GetR() + (float)(c2.GetR() - c1.GetR()) * t);
        BYTE g = (BYTE)(c1.GetG() + (float)(c2.GetG() - c1.GetG()) * t);
        BYTE b = (BYTE)(c1.GetB() + (float)(c2.GetB() - c1.GetB()) * t);
        return Color(a, r, g, b);
    }

    inline Color CurrentAccent()      { return LerpColor(kAccentMetro, kAccentRtx, g_accentTransition); }
    inline Color CurrentAccentHover() { return LerpColor(kAccentMetroHover, kAccentRtxHover, g_accentTransition); }
    inline Color CurrentAccentPress() { return LerpColor(kAccentMetroPress, kAccentRtxPress, g_accentTransition); }

    inline const Color kBtnNormal   (255, 70, 70, 75);
    inline const Color kBtnHover    (255, 100, 100, 105);
    inline const Color kBtnPress    (255, 130, 130, 135);

    inline const Color kBtnGreenNormal (255, 0, 168, 89);
    inline const Color kBtnGreenHover  (255, 0, 190, 100);
    inline const Color kBtnGreenPress  (255, 0, 140, 75);

    inline const Color kNavSelected (50, 255, 255, 255);

    inline const Color kTextPrimary (255, 235, 235, 240);
    inline const Color kTextMuted   (255, 148, 150, 165);
    inline const Color kTextConsole (255, 190, 40, 40);
    inline const Color kBorder      (255, 50, 50, 50);

    // ---- Инициализация/завершение GDI+ ----
    inline ULONG_PTR g_gdiplusToken = 0;
    inline PrivateFontCollection* g_pfc = nullptr;
    inline FontFamily* g_fontFamily = nullptr;
    inline HANDLE g_hFontMem = nullptr;

    inline void Init() {
        GdiplusStartupInput input;
        GdiplusStartup(&g_gdiplusToken, &input, nullptr);
        
        HRSRC hRes = FindResourceW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDR_FONT_MOSCOWSANS), (LPCWSTR)RT_RCDATA);
        if (hRes) {
            HGLOBAL hMem = LoadResource(GetModuleHandleW(nullptr), hRes);
            if (hMem) {
                void* data = LockResource(hMem);
                DWORD size = SizeofResource(GetModuleHandleW(nullptr), hRes);
                
                g_pfc = new PrivateFontCollection();
                g_pfc->AddMemoryFont(data, size);

                int count = g_pfc->GetFamilyCount();
                if (count > 0) {
                    FontFamily* pFamilies = new FontFamily[count];
                    int found = 0;
                    g_pfc->GetFamilies(count, pFamilies, &found);
                    if (found > 0) {
                        WCHAR familyName[LF_FACESIZE];
                        pFamilies[0].GetFamilyName(familyName);
                        g_fontFamily = new FontFamily(familyName, g_pfc);
                    }
                    delete[] pFamilies;
                }

                DWORD numFonts = 0;
                g_hFontMem = AddFontMemResourceEx(data, size, nullptr, &numFonts);
            }
        }

        if (!g_fontFamily || !g_fontFamily->IsAvailable()) {
            if (g_fontFamily) delete g_fontFamily;
            g_fontFamily = new FontFamily(L"Segoe UI");
        }
    }
    inline void Shutdown() {
        if (g_hFontMem) {
            RemoveFontMemResourceEx(g_hFontMem);
            g_hFontMem = nullptr;
        }
        if (g_fontFamily) {
            delete g_fontFamily;
            g_fontFamily = nullptr;
        }
        if (g_pfc) {
            delete g_pfc;
            g_pfc = nullptr;
        }
        GdiplusShutdown(g_gdiplusToken);
    }

    // ---- Вспомогательное: скруглённый прямоугольник ----
    inline void RoundedRectPath(GraphicsPath& path, const RectF& r, float radius) {
        float d = radius * 2;
        path.AddArc(r.X, r.Y, d, d, 180, 90);
        path.AddArc(r.X + r.Width - d, r.Y, d, d, 270, 90);
        path.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0, 90);
        path.AddArc(r.X, r.Y + r.Height - d, d, d, 90, 90);
        path.CloseFigure();
    }

    // Состояние кнопки для owner-draw (ховер/нажатие храним по HWND).
    struct ButtonVisualState {
        bool hovered = false;
        bool pressed = false;
        bool accent = false; // true = акцентная (главная) кнопка, false = обычная
        float animProgress = 0.0f; // от 0.0 (обычное) до 1.0 (наведено)
    };
    inline std::map<HWND, ButtonVisualState> g_buttonStates;

    // Плавная интерполяция двух цветов GDI+
    inline Color InterpolateColor(Color c1, Color c2, float t) {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return Color(
            255,
            (BYTE)(c1.GetR() + (c2.GetR() - c1.GetR()) * t),
            (BYTE)(c1.GetG() + (c2.GetG() - c1.GetG()) * t),
            (BYTE)(c1.GetB() + (c2.GetB() - c1.GetB()) * t)
        );
    }

    inline void DrawFlatButton(HDC hdc, RECT rc, const std::wstring& text, const ButtonVisualState& st, bool enabled = true) {
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        RectF r((REAL)rc.left, (REAL)rc.top, (REAL)(rc.right - rc.left), (REAL)(rc.bottom - rc.top));
        RectF inset(r.X + 0.5f, r.Y + 0.5f, r.Width - 1.0f, r.Height - 1.0f);
        GraphicsPath path;
        RoundedRectPath(path, inset, 6.0f);

        Color baseFill = st.accent ? CurrentAccent() : kBtnNormal;
        Color hoverFill = st.accent ? CurrentAccentHover() : kBtnHover;
        Color pressFill = st.accent ? CurrentAccentPress() : kBtnPress;

        Color fill = InterpolateColor(baseFill, hoverFill, st.animProgress);
        if (st.pressed) fill = pressFill;
        if (!enabled) {
            if (st.accent) {
                Color acc = CurrentAccent();
                fill = Color(150, acc.GetR() / 3, acc.GetG() / 3, acc.GetB() / 3);
            } else {
                fill = Color(40, 255, 255, 255);
            }
        }

        SolidBrush brush(fill);
        g.FillPath(&brush, &path);

        if (!st.accent) {
            Pen pen(kBorder, 1.0f);
            g.DrawPath(&pen, &path);
        }

        Font font(g_fontFamily, 10.0f, FontStyleBold, UnitPoint);
        Color textColor = enabled ? (st.accent ? Color(255, 255, 255, 255) : kTextPrimary) : kTextMuted;
        SolidBrush textBrush(textColor);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(text.c_str(), -1, &font, r, &format, &textBrush);
    }

    // Рисует скругленную кнопку (pill-shape) с прозрачным фоном и обводкой.
    inline void DrawPillButton(HDC hdc, RECT rc, const std::wstring& text, const ButtonVisualState& st, bool enabled = true, Color bgCol = kBgMain) {
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        
        g.Clear(bgCol); // Очищаем фон внутри единого объекта Graphics

        RectF r((REAL)rc.left, (REAL)rc.top, (REAL)(rc.right - rc.left), (REAL)(rc.bottom - rc.top));
        RectF inset(r.X + 0.5f, r.Y + 0.5f, r.Width - 1.0f, r.Height - 1.0f);
        GraphicsPath path;
        float radius = inset.Height / 2.0f;
        RoundedRectPath(path, inset, radius);

        Color baseFill = st.accent ? CurrentAccent() : kBtnNormal;
        Color hoverFill = st.accent ? CurrentAccentHover() : kBtnHover;
        Color pressFill = st.accent ? CurrentAccentPress() : kBtnPress;

        Color fill = InterpolateColor(baseFill, hoverFill, st.animProgress);
        if (st.pressed) fill = pressFill;
        if (!enabled) {
            fill = Color(40, 255, 255, 255);
        }

        SolidBrush brush(fill);
        g.FillPath(&brush, &path);

        if (!st.accent) {
            Pen pen(kBorder, 1.0f);
            g.DrawPath(&pen, &path);
        }

        Font font(g_fontFamily, 11.0f, FontStyleBold, UnitPoint);
        Color textColor = enabled ? Color(255, 255, 255, 255) : kTextMuted;
        SolidBrush textBrush(textColor);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(text.c_str(), -1, &font, r, &format, &textBrush);
    }

    // Рисует пункт навигации сайдбара (плоский, без рамки, с полосой слева при выборе).
    inline void DrawNavItem(HDC hdc, RECT rc, const std::wstring& text, bool selected, const ButtonVisualState& st, float indicatorLocalY = -1000.0f, float indicatorWidth = 3.0f) {
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        RectF r((REAL)rc.left, (REAL)rc.top, (REAL)(rc.right - rc.left), (REAL)(rc.bottom - rc.top));
        
        Color baseNavBg(0, 255, 255, 255); // Transparent normal
        Color hoverNavBg(20, 255, 255, 255);
        Color fill = selected ? kNavSelected : InterpolateColor(baseNavBg, hoverNavBg, st.animProgress);
        SolidBrush brush(fill);
        
        // Рисуем кружок для выбранного элемента в центре
        float radius = r.Width * 0.4f;
        if (radius > r.Height * 0.4f) radius = r.Height * 0.4f;
        RectF circleRect(r.X + r.Width / 2.0f - radius, r.Y + r.Height / 2.0f - radius, radius * 2.0f, radius * 2.0f);
        g.FillEllipse(&brush, circleRect);

        if (indicatorLocalY > -500.0f) {
            SolidBrush accentBrush(kBtnGreenNormal);
            g.FillRectangle(&accentBrush, RectF(r.X, indicatorLocalY, indicatorWidth, r.Height));
        }

        FontFamily iconFamily(L"Segoe MDL2 Assets");
        FontFamily fallbackFamily(L"Segoe UI Symbol");
        Font font(iconFamily.IsAvailable() ? &iconFamily : &fallbackFamily, 16.0f, FontStyleRegular, UnitPoint); // Большой шрифт для иконок
        SolidBrush textBrush(selected ? kTextPrimary : kTextMuted);
        StringFormat format;
        format.SetAlignment(StringAlignmentCenter);
        format.SetLineAlignment(StringAlignmentCenter);
        g.DrawString(text.c_str(), -1, &font, r, &format, &textBrush);
    }

    // Рисует пункт списка (без фиолетовой рамки, только легкая подсветка фона).
    inline void DrawListItem(HDC hdc, RECT rc, const std::wstring& text, bool selected, const ButtonVisualState& st) {
        Graphics g(hdc);
        g.SetSmoothingMode(SmoothingModeAntiAlias);
        g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

        RectF r((REAL)rc.left, (REAL)rc.top, (REAL)(rc.right - rc.left), (REAL)(rc.bottom - rc.top));
        
        Color baseListBg(0, 255, 255, 255);
        Color hoverListBg(15, 255, 255, 255);
        Color fill = selected ? kBgPanel : InterpolateColor(baseListBg, hoverListBg, st.animProgress);
        SolidBrush brush(fill);
        g.FillRectangle(&brush, r);

        Font font(g_fontFamily, 10.0f, FontStyleBold, UnitPoint);
        SolidBrush textBrush(selected ? kTextPrimary : kTextMuted);
        StringFormat format;
        format.SetAlignment(StringAlignmentNear);
        format.SetLineAlignment(StringAlignmentCenter);
        RectF textRect(r.X + 15.0f, r.Y, r.Width - 20.0f, r.Height);
        g.DrawString(text.c_str(), -1, &font, textRect, &format, &textBrush);
    }

    // Простая подпись HWND-подкласса для отслеживания hover (WM_MOUSEMOVE/WM_MOUSELEAVE).
    inline WNDPROC g_defaultButtonProc = nullptr;

    inline LRESULT CALLBACK HoverTrackSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_MOUSEMOVE: {
            auto& st = g_buttonStates[hwnd];
            if (!st.hovered) {
                st.hovered = true;
                SetTimer(hwnd, 1, 16, nullptr); // 60 FPS animation timer
                TRACKMOUSEEVENT tme = { sizeof(tme) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
            }
            break;
        }
        case WM_MOUSELEAVE: {
            auto& st = g_buttonStates[hwnd];
            st.hovered = false;
            st.pressed = false;
            SetTimer(hwnd, 1, 16, nullptr);
            break;
        }
        case WM_TIMER: {
            if (wParam == 1) {
                auto& st = g_buttonStates[hwnd];
                float target = st.hovered ? 1.0f : 0.0f;
                float step = 0.15f; // ~6-7 кадров для полного перехода (~100мс)
                bool changed = false;
                if (st.animProgress < target) {
                    st.animProgress += step;
                    if (st.animProgress >= target) { st.animProgress = target; KillTimer(hwnd, 1); }
                    changed = true;
                } else if (st.animProgress > target) {
                    st.animProgress -= step;
                    if (st.animProgress <= target) { st.animProgress = target; KillTimer(hwnd, 1); }
                    changed = true;
                }
                if (changed) InvalidateRect(hwnd, nullptr, FALSE);
            }
            break;
        }
        case WM_DESTROY: {
            KillTimer(hwnd, 1);
            break;
        }
        case WM_LBUTTONDOWN: {
            g_buttonStates[hwnd].pressed = true;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case WM_LBUTTONUP: {
            g_buttonStates[hwnd].pressed = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case WM_ERASEBKGND: {
            return 1;
        }
        }
        return CallWindowProcW(g_defaultButtonProc, hwnd, msg, wParam, lParam);
    }

    // Подключает отслеживание hover/press к owner-draw кнопке.
    inline void EnableHoverTracking(HWND hButton, bool accent = false) {
        g_buttonStates[hButton] = ButtonVisualState{ false, false, accent };
        WNDPROC prev = (WNDPROC)SetWindowLongPtrW(hButton, GWLP_WNDPROC, (LONG_PTR)HoverTrackSubclassProc);
        if (!g_defaultButtonProc) g_defaultButtonProc = prev;
    }

} // namespace Theme
