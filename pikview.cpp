// pikview.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "pikview.h"
#include "uicomponents.h"
#include "UIManager.h"
#include "GifHelper.h"

const static int PreloadPages = 1;
const static float ScrollRegionRatio = 0.4f;

struct ImageFile
{
	std::wstring Filename;
	struct Frame {
		sf::Texture Texture;
		sf::Sprite Sprite;
	};
	std::vector<Frame> Frames;
	int CurrentFrame;
	bool Loaded;
};

std::vector<ImageFile> InCurrentDirectory;
std::wstring OpenFileDir;
std::wstring OpenFileName;
int CurrentImageIndex = -1;

std::mutex M;
std::condition_variable FlippingNotifyer;

sf::Vector2u WindowRes;

enum ShowMode
{
	MODE_SCROLL,
	MODE_FIT,
	NUM_OF_MODE
};

int Mode = MODE_SCROLL;

void NextMode()
{
	if (Mode < NUM_OF_MODE - 1)
		Mode = Mode + 1;
	else
		Mode = 0;
}

ImageFile& GetCurrentImage()
{
	return InCurrentDirectory[CurrentImageIndex];
}

bool hasEnding(std::wstring const &fullString, std::wstring const &ending)
{
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
	}
	else {
		return false;
	}
}

bool FindImageFilesInDir(std::wstring path) {
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA ffd;
	std::wstring spec = path + L'*';

	InCurrentDirectory.clear();
	hFind = FindFirstFile(spec.c_str(), &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return false;
	}

	do {
		if (wcscmp(ffd.cFileName, L".") != 0 &&
			wcscmp(ffd.cFileName, L"..") != 0 &&
			!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::wstring file(ffd.cFileName);
			if (hasEnding(file, L".jpg") || hasEnding(file, L".bmp")
				|| hasEnding(file, L".png") || hasEnding(file, L".gif"))
			{
				ImageFile img;
				img.Filename = file;
				img.Loaded = false;
				InCurrentDirectory.push_back(img);
				ImageFile& b = InCurrentDirectory.back();
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	if (GetLastError() != ERROR_NO_MORE_FILES) {
		FindClose(hFind);
		return false;
	}

	FindClose(hFind);
	hFind = INVALID_HANDLE_VALUE;
	return true;
}

bool LoadTextureFromFile(std::vector<ImageFile::Frame>& List, std::wstring& path)
{
	if (hasEnding(path, L".gif"))
	{
		GifHelper gh;
		if (!gh.LoadFromFile(path))
			return false;
		int frames = gh.GetFrameCount();
		for (int i = 0; i < frames; i++)
		{
			List.push_back(ImageFile::Frame());
			List.back().Texture.create(gh.GetWidth(), gh.GetHeight());
			sf::FloatRect r = gh.GetFrameRect(i);
			List.back().Texture.update(gh.GetFrame(i), r.width, r.height, r.left, r.top);
		}
		return true;
	}
	else
	{
		std::ifstream f;
		f.open(path, std::wifstream::binary | std::wifstream::ate);
		if (!f.is_open())
			return false;
		int size = (int)f.tellg();
		f.seekg(0);
		char* buf = new char[size];
		f.read(buf, size);
		List.push_back(ImageFile::Frame());
		bool ret = List.back().Texture.loadFromMemory(buf, size);
		List.back().Texture.setSmooth(true);
		delete[] buf;
		return ret;
	}
}

int ParseCmdLine(LPWSTR Cmdline)
{
	int argc;
	wchar_t** argv = CommandLineToArgvW(Cmdline, &argc);
	if (argc == 0)
		return 0;
	wchar_t* ptr = wcslen(argv[argc - 1]) + argv[argc - 1];
	while (*ptr != L'\\')
		ptr--;
	OpenFileName = std::wstring(ptr + 1);
	ptr[1] = 0;
	OpenFileDir = std::wstring(argv[argc - 1]);
	return argc;
}

enum LoadResult
{
	LR_OK,
	LR_ERR,
	LR_LOADED
};

#define Images InCurrentDirectory

void AdjustSpriteToCenter(sf::Sprite& s, int mode=2)
{
	sf::Vector2f pos = s.getPosition();
	sf::FloatRect size = s.getLocalBounds();
	size.height *= s.getScale().x;
	size.width *= s.getScale().x;
	switch (mode)
	{
	case 0: // X
		pos.x = (WindowRes.x - size.width) / 2;
		break;
	case 1: // Y
		pos.y = (WindowRes.y - size.height) / 2;
		break;
	case 2: // Both
		pos.x = (WindowRes.x - size.width) / 2;
		pos.y = (WindowRes.y - size.height) / 2;
		break;
	default:
		break;
	}
	s.setPosition(pos);
}

LoadResult LoadImageByIndex(size_t i)
{
	if (Images[i].Loaded)
		return LR_LOADED;
	if (!LoadTextureFromFile(Images[i].Frames, OpenFileDir + Images[i].Filename))
	{
		return LR_ERR;
	}
	for (auto& Frame : Images[i].Frames)
	{
		Frame.Texture.setSmooth(false);
		Frame.Sprite.setTexture(Frame.Texture);
		AdjustSpriteToCenter(Frame.Sprite);
	}
	Images[i].Loaded = true;
	Images[i].CurrentFrame = 0;
	return LR_OK;
}

bool Running = true;
bool Notified = false;
int Skip = 0;

void IncImageIndex()
{
	if (CurrentImageIndex >= Images.size() - 1)
		//CurrentImageIndex = 0;
		return;
	else
	{
		CurrentImageIndex++;
		GetCurrentImage().CurrentFrame = 0;
		Skip = 0;
	}
	FlippingNotifyer.notify_one();
	Notified = true;
}

void DecImageIndex()
{
	if (CurrentImageIndex <= 0)
		//CurrentImageIndex = Images.size() - 1;
		return;
	else
	{
		CurrentImageIndex--;
		GetCurrentImage().CurrentFrame = 0;
		Skip = 0;
	}
	FlippingNotifyer.notify_one();
	Notified = true;
}

void PreloadThread()
{
	std::unique_lock<std::mutex> lock(M);
	while (Running)
	{

		for (int i = (int)CurrentImageIndex + 1; i <= CurrentImageIndex + PreloadPages; i++)
		{
			if (i >= Images.size())
				break;
			switch (LoadImageByIndex(i))
			{
			case LR_OK:
				break;
			case LR_LOADED:
				continue;
			case LR_ERR:
				throw;
			}
		}
		for (int i = (int)CurrentImageIndex - 1; i >= CurrentImageIndex - PreloadPages; i--)
		{
			if (i < 0)
				break;
			switch (LoadImageByIndex(i))
			{
			case LR_OK:
				break;
			case LR_LOADED:
				continue;
			case LR_ERR:
				throw;
			}
		}
		while (!Notified)
			FlippingNotifyer.wait_for(lock, std::chrono::milliseconds(200));
		Notified = false;
	}
}

bool InitApplication(LPWSTR Cmdline, sf::VideoMode& vm)
{
	if (!*Cmdline)
	{
		MessageBoxW(NULL, L"Must be launched with command line argument", L"PikView", MB_ICONINFORMATION);
		//MessageBoxW(NULL, lpCmdLine, L"PikView", MB_ICONINFORMATION);
		return false;
	}

	int argc = ParseCmdLine(Cmdline);
#ifdef _DEBUG
	sf::VideoMode screen_mode = vm = sf::VideoMode(1600, 900, 32);
#else
	sf::VideoMode screen_mode = vm = sf::VideoMode::getDesktopMode();
#endif
	WindowRes = { screen_mode.width,screen_mode.height };

	FindImageFilesInDir(OpenFileDir);
	for (auto it = InCurrentDirectory.begin(); it != InCurrentDirectory.end(); it++)
	{
		if (it->Filename == OpenFileName)
		{
			CurrentImageIndex = (int)std::distance(InCurrentDirectory.begin(), it);
			break;
		}
	}
	if (!LoadTextureFromFile(GetCurrentImage().Frames, OpenFileDir + OpenFileName))
	{
		MessageBoxW(NULL, (std::wstring(L"Failed to open file ") + OpenFileName).c_str(), L"Error", MB_ICONERROR);
		return false;
	}

	for (auto& Frame : GetCurrentImage().Frames)
	{
		//Frame.Texture.setSmooth(true);
		Frame.Sprite.setTexture(Frame.Texture);
		AdjustSpriteToCenter(Frame.Sprite);
	}
	GetCurrentImage().Loaded = true;
	GetCurrentImage().CurrentFrame = 0;

	return true;
}

void OpenConsoleForDebug()
{
#ifdef _DEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif
}

void UpdatePageNoDisplay(UIText*t)
{
	char buf[128];
	sprintf(buf, "%d/%d", CurrentImageIndex + 1, InCurrentDirectory.size());
	t->SetText(buf);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	OpenConsoleForDebug();
	sf::VideoMode screen_mode;
	if (!InitApplication(lpCmdLine, screen_mode))
		return 1;

	std::thread thread_preload(PreloadThread);

	sf::RenderWindow window(screen_mode, "PikView", sf::Style::None);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	UIManager ui(&window);
	UIButton* b1 = new UIButton(sf::FloatRect(WindowRes.x - 50.f, 0.f, 50.f, 50.f));
	b1->SetOnClickEvent([&] {window.close(); });
	ui.Add(b1);
	UIText* txtPgNo = new UIText(sf::FloatRect(20.f, WindowRes.y - 50.f, 100.f, 50.f), "aaa");
	ui.Add(txtPgNo);
	UpdatePageNoDisplay(txtPgNo);
	/*
	int drawcalled = 0;
	sf::Font f;
	f.loadFromFile("c:\\windows\\fonts\\consolab.ttf");
	*/
	bool NeedRedraw = true;
	bool NewEventDuringForceRedraw = false;
	float ScrollRatioX = 0.f;
	float ScrollRatioY = 0.f;
	while (window.isOpen())
	{
		sf::Event ev;
		//while ((NeedRedraw = ui.NeedRedraw()) || window.waitEvent(ev))
		while (window.pollEvent(ev))
		{
			ui.EventHandler(ev);
			switch (ev.type)
			{
			case sf::Event::KeyPressed:
				switch (ev.key.code)
				{
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Right:
					IncImageIndex();
					UpdatePageNoDisplay(txtPgNo);
					break;
				case sf::Keyboard::Left:
					DecImageIndex();
					UpdatePageNoDisplay(txtPgNo);
					break;
				default:
					break;
				}
				break;
			case sf::Event::MouseWheelMoved:
				if (ev.mouseWheel.delta > 0)
				{
					IncImageIndex();
					UpdatePageNoDisplay(txtPgNo);
				}
				else
				{
					DecImageIndex();
					UpdatePageNoDisplay(txtPgNo);
				}
				break;
			case sf::Event::MouseButtonPressed:
				switch (ev.mouseButton.button)
				{
				case sf::Mouse::Left:
					IncImageIndex();
					UpdatePageNoDisplay(txtPgNo);
					FlippingNotifyer.notify_one();
					break;
				case sf::Mouse::Right:
					DecImageIndex();
					UpdatePageNoDisplay(txtPgNo);
					FlippingNotifyer.notify_one();
					break;
				case sf::Mouse::Middle:
					NextMode();
					Skip = 0;
					break;
				case sf::Mouse::XButton2:
					window.close();
					break;
				default:
					break;
				}
				break;
			case sf::Event::MouseMoved:
			{
				if (!GetCurrentImage().Loaded)
					break;
				sf::FloatRect ImgRect = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.getLocalBounds();
				if (ImgRect.height < WindowRes.y&&ImgRect.width < WindowRes.x)
					break;
				float HaltRegionRatio = (1 - ScrollRegionRatio) / 2;
				int SRUpperBound = (int)(WindowRes.y * HaltRegionRatio);
				int SRLowerBound = (int)(WindowRes.y * (1 - HaltRegionRatio));
				int SRLeftBound = (int)(WindowRes.x * HaltRegionRatio);
				int SRRightBound = (int)(WindowRes.x * (1 - HaltRegionRatio));
				if (ev.mouseMove.y <SRUpperBound)
				{
					ScrollRatioY = 0.f;
				}
				else if (ev.mouseMove.y>SRLowerBound)
				{
					ScrollRatioY = 1.f;
				}
				else
				{
					ScrollRatioY = ((float)ev.mouseMove.y - SRUpperBound) / (SRLowerBound - SRUpperBound);
				}
				if (ev.mouseMove.x <SRLeftBound)
				{
					ScrollRatioX = 0.f;
				}
				else if (ev.mouseMove.x>SRRightBound)
				{
					ScrollRatioX = 1.f;
				}
				else
				{
					ScrollRatioX = ((float)ev.mouseMove.x - SRLeftBound) / (SRRightBound - SRLeftBound);
				}
				break;
			}
			case sf::Event::Closed:
				window.close();
				break;
			default:
				break;
			}
		}
		if(GetCurrentImage().CurrentFrame==0)
			window.clear(sf::Color::Black);
		if (GetCurrentImage().Loaded) {
			sf::FloatRect ImgRect = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.getLocalBounds();
			switch (Mode)
			{
			case MODE_SCROLL:
				GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setScale(1.f, 1.f);
				if (ImgRect.height > WindowRes.y&&ImgRect.width > WindowRes.x)
				{
					sf::Vector2f pos = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.getPosition();
					pos.x = (WindowRes.x - ImgRect.width) * ScrollRatioX;
					pos.y = (WindowRes.y - ImgRect.height) * ScrollRatioY;
					GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setPosition(pos);
				}
				else if (ImgRect.width > WindowRes.x)
				{
					sf::Vector2f pos = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.getPosition();
					pos.x = (WindowRes.x - ImgRect.width) * ScrollRatioX;
					GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setPosition(pos);
					AdjustSpriteToCenter(GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite, 1);
				}
				else if (ImgRect.height > WindowRes.y)
				{
					sf::Vector2f pos = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.getPosition();
					pos.y = (WindowRes.y - ImgRect.height) * ScrollRatioY;
					GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setPosition(pos);
					AdjustSpriteToCenter(GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite, 0);
				}
				else
				{
					AdjustSpriteToCenter(GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite, 2);
				}
				break;
			case MODE_FIT:
			{
				float ImgRatio = ImgRect.width / ImgRect.height;
				float WndRatio = (float)WindowRes.x / WindowRes.y;
				if (ImgRatio < WndRatio)
				{
					sf::Vector2u TexSize = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Texture.getSize();
					float ZoomRatio = (float)WindowRes.y / TexSize.y;
					GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setScale(ZoomRatio, ZoomRatio);
				}
				else
				{
					sf::Vector2u TexSize = GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Texture.getSize();
					float ZoomRatio = (float)WindowRes.x / TexSize.x;
					GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite.setScale(ZoomRatio, ZoomRatio);
				}
				AdjustSpriteToCenter(GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite, 2);
				break;
			}
			default:
				break;
			}
			window.draw(GetCurrentImage().Frames[GetCurrentImage().CurrentFrame].Sprite);

			if (GetCurrentImage().Frames.size() > 1 && Skip % 3 == 2)
			{
				Skip = 0;
				if (GetCurrentImage().CurrentFrame < GetCurrentImage().Frames.size() - 1)
					GetCurrentImage().CurrentFrame++;
				else
					GetCurrentImage().CurrentFrame = 0;
			}
			Skip++;
		}
		else
		{

		}
		/*
		sf::Text frames(std::to_string(drawcalled), f, 60);
		frames.setColor(sf::Color::Red);
		drawcalled++;
		window.draw(frames);
		*/
		ui.Update(0.f);
		ui.Draw();
		window.display();
	}
	Running = false;
	Notified = true;
	FlippingNotifyer.notify_one();
	thread_preload.join();
	return 0;
}

