#include "stdafx.h"
#include "Quicklinks32.h"

#define MAX_LOADSTRING 100

const int SCREENW = 500;
const int SCREENH = 538;

const int inactiveRed = 204;
const int inactiveGreen = 183;
const int inactiveBlue = 91;

const int activeRed = 164;
const int activeGreen = 143;
const int activeBlue = 71;

/*
const int inactiveRed = 140;
const int inactiveGreen = 60;
const int inactiveBlue = 180;

const int activeRed = 160;
const int activeGreen = 80;
const int activeBlue = 200;
*/

const int clickedRed = 120;
const int clickedGreen = 40;
const int clickedBlue = 160;

const int numberOfButtons = 26; 

//Global Variables
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HBITMAP hBitmap = NULL;
HBITMAP hBitmap2 = NULL;
HBITMAP hBitmap3 = NULL;

static HFONT s_hFont = NULL;
static HFONT big_hFont = NULL;

extern inline bool ExistTest(const std::string& name);

//GAME
extern int start; //timing integer.
extern RECT windowRect;
byte tetris = 0;
bool tetrisEnable = false; //SET TO FALSE BEFORE FINAL RELEASE
bool showGame = false; //set to false before final release
bool gameInit = false;
bool gameover = false; //needed to run

bool jimsMom = false;
byte jims = 0;

bool adminEnable = false;
byte admin = 0;

//TODO:
/*

*/

using namespace std;

char msg[32];
char msgLink[602];
string buttons[numberOfButtons + 1] = { "John Cena",
"Button Caption",
"Hello,",
"Please",
"Copy",
"The",
"Shortcut",
"Instead",
"To",
"Use",
"Public",
"Quick",
"Links",
"Or",
"Edit",
"The",
"Config",
"File",
"To",
"Have",
"Your",
"Own",
"Private",
"Quick",
"Links",
"Thank",
"You" //add new button names here
};

string links[numberOfButtons + 1] = { "Link0",
"https://www.google.com",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL",
"NULL"//add new button links here
};



class custombtn
{
public:
	BOOL inbox = false;
	BOOL wasinbox = false;
	BOOL mousedowninbox = false;
	BOOL mousecurrentlyup = true;
	BOOL wasClicked = false;
	RECT btnrect;
	LPSTR btncaption;
	LPSTR btnlink;

	BOOL mouseonbtn(int mposx, int mposy)
	{
		if (mposx >= btnrect.left && mposy >= btnrect.top && mposx <= btnrect.right && mposy <= btnrect.bottom)
			return true;
		else
			return false;
	}


	VOID drawbtn(HDC hdc, HBRUSH myhBrush)
	{
		RECT rect;
		int textheight;

		if (wasClicked)
		{
			HBRUSH clickedBrush = CreateSolidBrush(RGB(clickedRed, clickedGreen, clickedBlue));
			FillRect(hdc, &btnrect, clickedBrush);
			DeleteObject(clickedBrush);
		}
		else
		{
			FillRect(hdc, &btnrect, myhBrush);
		};

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));

		rect = btnrect;
		textheight = DrawTextA(hdc, btncaption, -1, &rect, DT_CALCRECT);
		rect = btnrect;
		rect.top = ((rect.bottom + rect.top) / 2) - (textheight / 2);
		DrawText(hdc, btncaption, -1, &rect, DT_CENTER);
	}

	VOID detectmouseover(HWND hWnd, int x, int y)
	{
		wasinbox = inbox;

		if (wasClicked)
		{
			wasClicked = false;
			InvalidateRect(hWnd, &btnrect, FALSE);
		};

		if (mouseonbtn(x, y))
		{
			inbox = true;
			if (wasinbox != inbox)
			{
				InvalidateRect(hWnd, &btnrect, FALSE);
			};
		}
		else
		{
			inbox = false;
			if (wasinbox != inbox)
			{
				InvalidateRect(hWnd, &btnrect, FALSE);
			};
		};
	}

	VOID mousedown(int x, int y)
	{
		if (inbox && mousecurrentlyup)
		{
			mousedowninbox = true;
		}
		else
		{
			mousedowninbox = false;
		};
		mousecurrentlyup = false;
	}

	VOID mouseup(HWND hWnd, int x, int y)
	{
		if (inbox && mousedowninbox)
		{
			ShellExecute(NULL, "open", btnlink, NULL, NULL, SW_SHOWNORMAL);
			InvalidateRect(hWnd, &btnrect, FALSE);
			wasClicked = true;
		};
		mousedowninbox = false;
		mousecurrentlyup = true;
	}

	custombtn(int btnx1, int btny1, int btnx2, int btny2, LPSTR caption, LPSTR link)
	{
		btnrect.top = btny1 - 1;
		btnrect.left = btnx1 - 1;
		btnrect.bottom = btny2 - 1;
		btnrect.right = btnx2 - 1;
		btnlink = link;
		btncaption = caption;
	}

	custombtn()
	{

	}
};


ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Caption(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Link(HWND, UINT, WPARAM, LPARAM);

void MakeConfig(string buttonName[], string linkName[])
{
	int pos;
	string stringLabel;

	ofstream outfile;
	outfile.open("config.cfg");

	if (!outfile.is_open())
		MessageBox(0, "Changes were not saved to config.cfg\n\nPlease try again if you wish to make this change permanent.", 0, 0);
	else
	{
		for (int i = 1; i <= numberOfButtons; i = i++)
		{
			stringLabel = buttonName[i];
			while (true)
			{
				pos = stringLabel.find("\n");
				if (pos != std::string::npos)
				{
					stringLabel.replace(pos, 1, "/n");
				}
				else
				{
					break;
				}
			}


			outfile << stringLabel << "\n" << linkName[i] << "\n";
		};
		outfile.close();
	};
};

custombtn btn[numberOfButtons + 1];

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);




	if (ExistTest("config.cfg"))
	{
		int pos, i;
		i = 1;
		string inStringLabel, inStringLink;

		ifstream infile;
		infile.open("config.cfg");

		infile.clear();
		while (!infile.eof())
		{
			getline(infile, inStringLabel); //infile >> inStringLabel;
			do
			{
				pos = inStringLabel.find("/n");
				if (pos != std::string::npos)
				{
					inStringLabel.replace(pos, 2, "\n");
					infile.clear();
				}
				else
				{
					break;
				};
			} while (pos != std::string::npos); //this should be this... pos != std::string::npos
			buttons[i] = inStringLabel;
			getline(infile, links[i]);//infile >> links[i];
			i++;
			if (i > numberOfButtons)
				break;
		};
		infile.close();
	}
	else
	{
		MakeConfig(buttons, links);
	};



	btn[1].btnrect = { 130, 57, 477, 117 };
	btn[1].btnlink = (LPSTR)links[1].c_str();
	btn[1].btncaption = (LPSTR)buttons[1].c_str();

	btn[2].btnrect = { 130, 122, 477, 182 };
	btn[2].btnlink = (LPSTR)links[2].c_str();
	btn[2].btncaption = (LPSTR)buttons[2].c_str();



	int k = 3;
	for (int j = 0; j < (numberOfButtons - 2) / 3; j = j + 1)
	{
		for (int i = 0; i <= 2; i = i + 1)
		{
			btn[k].btnrect = { 7 + (158 * i), 194 + (38 * j), 160 + (158 * i), 228 + (38 * j) };
			btn[k].btnlink = (LPSTR)links[k].c_str();
			btn[k].btncaption = (LPSTR)buttons[k].c_str();
			k++;
		};
	};



	tetris = 0;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_QUICKLINKS32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE,
		CW_USEDEFAULT, 0, SCREENW, SCREENH, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	};

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_QUICKLINKS32));

	MSG msg;

	while (!gameover)
	{
		if (showGame == false)
		{
			while (GetMessage(&msg, nullptr, 0, 0))
			{
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				};
				if (showGame == true)
				{
					break;
				};
			};

		}
		else
		{
			start = clock();
			while (start + 20 > clock()) // 1000/20 = 50 ticks/second max.
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) //Gets inputs while the game is waiting for max Updates per second wait time.
				{
					if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					};
				};
				Sleep(1);
			};
			Game_Run(hWnd);
		};
	};

	return (int)msg.wParam;
};

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON3));


	return RegisterClassEx(&wcex);
};


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{

		hBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));
		if (hBitmap == NULL)
			MessageBox(NULL, "Background Banner didn't load!!", "Error!", MB_ICONEXCLAMATION | MB_OK);

		hBitmap2 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1));
		if (hBitmap2 == NULL)
			MessageBox(NULL, "Background Sq. Logo didn't load!!", "Error!", MB_ICONEXCLAMATION | MB_OK);


		break;

	};


	case WM_CONTEXTMENU:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);
		HMENU hMenu = ::CreatePopupMenu();
		if (NULL != hMenu)
		{

			for (int m = 1; m <= numberOfButtons; m++)
			{
				if (btn[m].inbox)
				{
					if (adminEnable)
					{
						::AppendMenu(hMenu, MF_STRING, 1, "Change Caption");
						::AppendMenu(hMenu, MF_STRING, 2, "Change Link");
					}
					else
					{
						::AppendMenu(hMenu, MF_STRING | MF_GRAYED, 1, "Change Caption");
						::AppendMenu(hMenu, MF_STRING | MF_GRAYED, 2, "Change Link");
					};
				};
			};

			::AppendMenu(hMenu, MF_STRING, 3, "About...");

			int sel = ::TrackPopupMenuEx(hMenu,
				TPM_LEFTALIGN | TPM_RETURNCMD,
				xPos,
				yPos,
				hWnd,
				NULL);

			switch (sel)
			{
			case 1:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_CAPTIONBOX), hWnd, Caption);

				HWND caption = CreateWindow(szWindowClass, "Caption", WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
					CW_USEDEFAULT, 0, 100, 100, hWnd, nullptr, NULL, nullptr);
				break;
			};

			case 2:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_LINKBOX), hWnd, Link);

				HWND link = CreateWindow(szWindowClass, "Link", WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
					CW_USEDEFAULT, 0, 100, 100, hWnd, nullptr, NULL, nullptr);
				break;
			};

			case 3:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);

				HWND about = CreateWindow(szWindowClass, "About", WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
					CW_USEDEFAULT, 0, 100, 100, hWnd, nullptr, NULL, nullptr);
				break;
			};
			};

			::DestroyMenu(hMenu);
		};
		break;

	};

	case WM_PAINT:
	{
		if (!showGame)
		{
			PAINTSTRUCT ps;
			BITMAP      bitmap;
			HDC         hdcMem, bmMem;
			HGDIOBJ     bitmapMem;
			HDC hdc = BeginPaint(hWnd, &ps);

			HPEN hBlackPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
			HBRUSH activeBrush = CreateSolidBrush(RGB(activeRed, activeGreen, activeBlue));
			HBRUSH inactiveBrush = CreateSolidBrush(RGB(inactiveRed, inactiveGreen, inactiveBlue));

			hdcMem = CreateCompatibleDC(hdc);
			bmMem = CreateCompatibleDC(hdc);
			bitmapMem = CreateCompatibleBitmap(hdc, SCREENW, SCREENH);

			GetObject(hBitmap, sizeof(bitmap), &bitmap);

			SelectObject(hdcMem, bitmapMem);

			SelectObject(bmMem, hBitmap);
			BitBlt(hdcMem, 147, 1, bitmap.bmWidth, bitmap.bmHeight, bmMem, 0, 0, SRCCOPY);


			if (!jimsMom)
			{
				BITMAP bitmap2;
				GetObject(hBitmap2, sizeof(bitmap2), &bitmap2);
				SelectObject(bmMem, hBitmap2);
				BitBlt(hdcMem, 1, 1, bitmap2.bmWidth, bitmap2.bmHeight, bmMem, 0, 0, SRCCOPY);
			}
			else
			{

				//RECT rect = { 1, 1, 147, 65 };

				//HBRUSH clickedBrush = CreateSolidBrush(RGB(53, 93, 174)); //color of sky from banner
				//FillRect(hdcMem, &rect, clickedBrush);
				//DeleteObject(clickedBrush);

				for (int i = 0; i < 143; i++)
				{
					BitBlt(hdcMem, i + 1, 1, 4, 70, bmMem, 0, 0, SRCCOPY);
				}

				BITMAP bitmap3;
				GetObject(hBitmap3, sizeof(bitmap3), &bitmap3);
				SelectObject(bmMem, hBitmap3);
				BitBlt(hdcMem, 1, 56, bitmap3.bmWidth, bitmap3.bmHeight, bmMem, 0, 0, SRCCOPY);
			}

			const TCHAR* fontName = _T("Arial");
			long nFontSize = 10;
			LOGFONT logFont = { 0 };
			logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdcMem, LOGPIXELSY), 72);
			logFont.lfWeight = FW_BOLD;
			_tcscpy_s(logFont.lfFaceName, fontName);
			s_hFont = CreateFontIndirect(&logFont);

			nFontSize = 22;
			logFont = { 0 };
			logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdcMem, LOGPIXELSY), 72);
			logFont.lfWeight = FW_BOLD;
			_tcscpy_s(logFont.lfFaceName, fontName);
			big_hFont = CreateFontIndirect(&logFont);

			for (int m = 1; m <= numberOfButtons; m = m + 1)
				if (!btn[m].inbox)
				{
					if (m == 1 || m == 2)
					{
						SelectObject(hdcMem, big_hFont); //big font for big buttons.
						btn[m].drawbtn(hdcMem, inactiveBrush);
						SelectObject(hdcMem, s_hFont);
					}
					else
					{
						btn[m].drawbtn(hdcMem, inactiveBrush);
					};
				}
				else
				{
					if (m == 1 || m == 2)
					{
						SelectObject(hdcMem, big_hFont); //big font for big buttons.
						btn[m].drawbtn(hdcMem, activeBrush);
						SelectObject(hdcMem, s_hFont);
					}
					else
					{
						btn[m].drawbtn(hdcMem, activeBrush);
					};
				};

			BitBlt(hdc, 0, 0, SCREENW, SCREENH, hdcMem, 0, 0, SRCCOPY);


			DeleteObject(activeBrush);
			DeleteObject(inactiveBrush);
			DeleteObject(bitmapMem);
			DeleteDC(hdcMem);
			DeleteDC(bmMem);
			DeleteObject(s_hFont);
			DeleteObject(big_hFont);
			DeleteDC(hdc);
			EndPaint(hWnd, &ps);
		}
		else
		{
			PAINTSTRUCT ps;
			HDC hdcMem;
			HDC hdc = BeginPaint(hWnd, &ps);
			HGDIOBJ bitmapMem;

			hdcMem = CreateCompatibleDC(hdc);

			bitmapMem = CreateCompatibleBitmap(hdc, SCREENW, SCREENH);

			SelectObject(hdcMem, bitmapMem);

			myGrid.DrawBlocks(hdcMem); //Prevents flashing if I call this in WM_PAINT

			BitBlt(hdc, 0, 0, SCREENW, SCREENH, hdcMem, 0, 0, SRCCOPY);

			DeleteObject(bitmapMem);

			DeleteDC(hdcMem);
			DeleteDC(hdc);
			EndPaint(hWnd, &ps);
		};
		break;
	};

	case WM_LBUTTONDOWN:
	{
		if (!showGame)
		{
			WORD x, y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			for (int m = 1; m <= numberOfButtons; m = m + 1)
				btn[m].mousedown(x, y);
		};
		break;
	};

	case WM_LBUTTONUP:
	{
		if (!showGame)
		{
			WORD x, y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			for (int m = 1; m <= numberOfButtons; m = m + 1)
				btn[m].mouseup(hWnd, x, y);
		};
		break;
	};

	case WM_MOUSEMOVE:
	{
		if (!showGame)
		{
			WORD x, y;
			x = LOWORD(lParam);
			y = HIWORD(lParam);

			for (int m = 1; m <= numberOfButtons; m = m + 1)
				btn[m].detectmouseover(hWnd, x, y);
		};
		break;
	};

	case WM_KEYDOWN:
	{
		if (!showGame)
		{
			WORD key;
			key = LOWORD(wParam);

			if (!tetrisEnable)
			{
				switch (tetris)
				{
				case 0:
				{
					if (key == 0x4D) //M
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 1:
				{
					if (key == 0x41) //A
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 2:
				{
					if (key == 0x4E) //N
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 3:
				{
					if (key == 0x44) //D
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 4:
				{
					if (key == 0x41) //A
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 5:
				{
					if (key == 0x54) //T 
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 6:
				{
					if (key == 0x4F) //O
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 7:
				{
					if (key == 0x52) //R
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 8:
				{
					if (key == 0x59) //Y
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 9:
				{
					if (key == 0x46) //F
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 10:
				{
					if (key == 0x55) //U
						tetris++;
					else
						tetris = 0;
					break;
				};
				case 11:
				{
					if (key == 0x4E) //N
						tetrisEnable = true;
					else
						tetris = 0;
					break;
				};
				default:
				{
					tetris = 0;
					break;
				};
				};
			};
			if (!adminEnable)
			{
				switch (admin)
				{
				case 0:
				{
					if (key == 0x4C) //L
						admin++;
					else
						admin = 0;
					break;
				};
				case 1:
				{
					if (key == 0x42) //B
						admin++;
					else
						admin = 0;

					if (key == 0x4C) //0x4C = L This is a hack in case someone types "LLB4L" by mistake.
						admin = 1;
					break;
				};
				case 2:
				{
					if (key == 0x34) //4
						admin++;
					else
						admin = 0;
					break;
				};
				case 3:
				{
					if (key == 0x4C) //L
						adminEnable = true;
					else
						admin = 0;
					break;
				};
				default:
				{
					admin = 0;
					break;
				};
				};
			};
			switch (jims)
			{
			case 0:
			{
				if (key == 0x4A) //J
					jims++;
				else
					jims = 0;
				break;
			}
			case 1:
			{
				if (key == 0x49) //I
					jims++;
				else
					jims = 0;
				break;
			}
			case 2:
			{
				if (key == 0x4D) //M
					jims++;
				else
					jims = 0;
				break;
			}
			case 3:
			{
				if (key == 0x53) //S
					jims++;
				else
					jims = 0;
				break;
			}
			case 4:
			{
				if (key == 0x4D) //M
					jims++;
				else
					jims = 0;
				break;
			}
			case 5:
			{
				if (key == 0x4F) //O
					jims++;
				else
					jims = 0;
				break;
			}
			case 6:
			{
				if (key == 0x4D) //M
				{
					RECT rect = { 0, 0, SCREENW, SCREENH };
					if (!jimsMom)
					{
						if(hBitmap3 == NULL)
							hBitmap3 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3));
						if (hBitmap3 == NULL)
							MessageBox(NULL, "Joke didn't load!!", "Error!", MB_ICONEXCLAMATION | MB_OK);
						else
						{
							jimsMom = true;
							InvalidateRect(hWnd, &rect, false);
						}
					}
					else
					{
						jimsMom = false;
						InvalidateRect(hWnd, &rect, false);
					}
					jims = 0;
				}
				else
					jims = 0;
				break;
			}
			};
			if (key == VK_ESCAPE && tetrisEnable)
			{
				if (!gameInit)
				{
					gameInit = Game_Init();
				};
				showGame = true;
				//Sleep(200); //prevents escape from being registered by the game loop
			};
		}
		else
		{
			WORD key; //key pressed.
			key = LOWORD(wParam);

			myGrid.UserInput(key);
		};
		break;
	};

	case WM_KILLFOCUS:
	{
		if (gameInit)
		{
			myGrid.Pause();
			showGame = false; //Hide the game because the user is focused on something else.
		};

		break;
	};

	case WM_DESTROY:
	{

		DeleteObject(hBitmap);
		DeleteObject(hBitmap2);
		DeleteObject(hBitmap3);
		DeleteObject(s_hFont);
		DeleteObject(big_hFont);
		PostQuitMessage(0);
		gameover = true;
		break;
	};

	default:
	{
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	};
	};
	return 0;
};

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		return (INT_PTR)TRUE;
		break;
	};

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		};
		break;
	};
	return (INT_PTR)FALSE;
};

INT_PTR CALLBACK Caption(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		int pos;
		for (int m = 1; m <= numberOfButtons; m = m + 1)
		{
			string workingString = buttons[m];
			if (btn[m].inbox)
			{
				do
				{
					pos = workingString.find("\n");
					if (pos != std::string::npos)
					{
						workingString.replace(pos, 1, "/n");
					};
				} while (pos != std::string::npos);
				SetDlgItemTextA(hDlg, IDD_CAPTION_FIELD, (LPSTR)workingString.c_str());
			};
		};
		return (INT_PTR)TRUE;
		break;
	};

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			int pos;
			GetDlgItemText(hDlg, IDD_CAPTION_FIELD, (LPSTR)msg, 31);
			for (int m = 1; m <= numberOfButtons; m++)
			{
				if (btn[m].inbox)
				{
					buttons[m] = msg; //Change buttons first and then btncaption to buttons to get the pointer pointing at the string.
					do
					{
						pos = buttons[m].find("/n");
						if (pos != std::string::npos)
						{
							buttons[m].replace(pos, 2, "\n");
						};
					} while (pos != std::string::npos);
					btn[m].btncaption = (LPSTR)buttons[m].c_str();
					MakeConfig(buttons, links);
				};
			};
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//do nothing.
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		};
		break;
	};
	return (INT_PTR)FALSE;
};

INT_PTR CALLBACK Link(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		int pos;

		for (int m = 1; m <= numberOfButtons; m = m + 1)
		{
			if (btn[m].inbox)
			{
				SetDlgItemTextA(hDlg, IDD_LINK_FIELD, btn[m].btnlink);
			};
		};
		return (INT_PTR)TRUE;
		break;
	};

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDD_LINK_FIELD, (LPSTR)msgLink, 602);
			if ((unsigned)strlen(msgLink) < 600)
			{
				for (int m = 1; m <= numberOfButtons; m++)
				{
					if (btn[m].inbox)
					{
						links[m] = msgLink; //Change links first and then btnlink to links to get the pointer pointing at the string.
						btn[m].btnlink = (LPSTR)links[m].c_str();
						MakeConfig(buttons, links);
					};
				};
			}
			else
				MessageBox(hDlg, "The link can't be longer than 600 characters.", "Error: Link too Long", 0);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			//do nothing
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		};
		break;
	};
	return (INT_PTR)FALSE;
};