#include <windows.h>
#include <wchar.h>
#include "BaseApp.h"
#include "DPIScale.h"

BaseApp::BaseApp() : d2dFactory(NULL), renderingTarget(NULL), wicImaingFactory(NULL), d2dDWriteFactory(NULL)
{
}

BaseApp::~BaseApp()
{
	for (auto spriteSheet : spriteSheets)
		SafeRelease(&spriteSheet);

	for (auto brush : solidBrushes)
		SafeRelease(&brush);

	for (auto textFormat : textFormats)
		SafeRelease(&textFormat);

	SafeRelease(&d2dDWriteFactory);
	SafeRelease(&wicImaingFactory);
	SafeRelease(&d2dFactory);
	SafeRelease(&renderingTarget);
}


HRESULT BaseApp::createDeviceIndependentResources()
{
	HRESULT hr = S_OK;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);

	// Create a DirectWrite factory.
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&d2dDWriteFactory));

	// Create WIC Imaging factory.
	hr = CoCreateInstance(CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory),
		(void**)(&wicImaingFactory)
		);

	return hr;
}

HRESULT BaseApp::createDeviceResources(HWND hWnd)
{

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);

	D2D1_SIZE_U size = D2D1::SizeU(
		rc.right - rc.left,
		rc.bottom - rc.top
		);

	hr = d2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hWnd, size),
		&renderingTarget
		);

	size_t index{ 0 };

	// Brushes
	for (auto color : brushColor) {
		solidBrushes.push_back(NULL);
		hr = renderingTarget->CreateSolidColorBrush(
			color,
			&solidBrushes.at(index));
		++index;
	}

	// Text formats
	// Pasing in Font, font-size
	index = 0;
	for (auto font : fonts) {
		for (auto fontSize : fontSizes) {
			textFormats.push_back(NULL);
			hr = d2dDWriteFactory->CreateTextFormat(
				TUtils::toWSString(font).c_str(),
				NULL,
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				fontSize,
				L"en-us",
				&textFormats.at(index)
				);
			// Center align (horizontally) the text.
			if (SUCCEEDED(hr))
			{
				hr = textFormats.at(index)->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				hr = textFormats.at(index)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}
			++index;
		}
	}

	//// Bitmaps

	IWICBitmapDecoder* wicBitmapDecoder = NULL;
	IWICFormatConverter* wicFormatConverter = NULL;
	IWICBitmapFrameDecode* wicBitmapFrameDecode = NULL;

	index = 0;

	for (auto spriteSheetFile : spriteSheetFiles) {

		hr = wicImaingFactory->CreateDecoderFromFilename(
			TUtils::toWSString(spriteSheetFile).c_str(),
			NULL,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			&wicBitmapDecoder);

		hr = wicImaingFactory->CreateFormatConverter(&wicFormatConverter);

		hr = wicBitmapDecoder->GetFrame(0, &wicBitmapFrameDecode);

		hr = wicFormatConverter->Initialize(
			wicBitmapFrameDecode,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL, 0.f,
			WICBitmapPaletteTypeMedianCut
			);

		spriteSheets.push_back(NULL);

		hr = renderingTarget->CreateBitmapFromWicBitmap(
			wicFormatConverter,
			NULL,
			&spriteSheets.at(index)
			);

		++index;
	}


	return hr;
}

HRESULT BaseApp::initialize()
{
	HRESULT hr;

	//get the dpi information
	HDC screen = GetDC(0);
	int dpiX = GetDeviceCaps(screen, LOGPIXELSX);
	int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
	ReleaseDC(0, screen);

	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = createDeviceIndependentResources();

	DPIScale::Initialize(d2dFactory);

	if (SUCCEEDED(hr))
	{

		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = BaseApp::wndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName = NULL;
		wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = "Base App Window Class";

		RegisterClassEx(&wcex);

		// Create the window.
		mainWindowHandle = CreateWindow(
			"Base App Window Class",
			APP_WINDOW_CAPTION,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<INT>(APP_WINDOW_WIDTH * DPIScale::getScaleX()), // For modern screen
			static_cast<INT>(APP_WINDOW_HEIGHT * DPIScale::getScaleY()), // For modern screen
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = mainWindowHandle ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			loadConfig();
			hr = createDeviceResources(mainWindowHandle);
			initializeGameAssets();
			ShowWindow(mainWindowHandle, SW_SHOWNORMAL);
			UpdateWindow(mainWindowHandle);
		}

	}

	return hr;
}

int WINAPI WinMain(
	HINSTANCE /* hInstance */,
	HINSTANCE /* hPrevInstance */,
	LPSTR /* lpCmdLine */,
	int /* nCmdShow */
	)
{
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			BaseApp app;

			if (SUCCEEDED(app.initialize()))
			{
				app.runMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}

LRESULT CALLBACK BaseApp::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		BaseApp *pBaseApp = (BaseApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pBaseApp)
			);

		result = 1;
	}
	else
	{
		BaseApp *pBaseApp = reinterpret_cast<BaseApp *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		if (pBaseApp)
		{
			switch (message)
			{
			case WM_KEYDOWN:
				pBaseApp->onKeyDown(wParam);
				InvalidateRect(hwnd, NULL, FALSE);
				break;
			case WM_SIZE:
				pBaseApp->onResize(width, height);
				result = 0;
				wasHandled = true;
				break;
			case WM_DISPLAYCHANGE:
				InvalidateRect(hwnd, NULL, FALSE);
				result = 0;
				wasHandled = true;
				break;
			case WM_PAINT:
				pBaseApp->onRender();
				ValidateRect(hwnd, NULL);
				result = 0;
				wasHandled = true;
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				result = 1;
				wasHandled = true;
				break;

			case WM_LBUTTONDOWN:
				pBaseApp->onLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				break;

			case WM_LBUTTONUP:
				pBaseApp->onLButtonUp();
				break;

			case WM_MOUSEMOVE:
				pBaseApp->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

void BaseApp::runMessageLoop()
{
	MSG msg;

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Background sound
		// 
		//if (gameSound.BackgroundChanged())
		//	gameSound.playLoop(0, 0.5f);

		// Handle your timer related here
		gameTime.update();
		if (gameTime.getElapsedTime() > SCREEN_UPDATE) {

			// Update your timer related objects
			for (auto npc : npcs) {
				npc->update(gameTime);
			}

			for (auto npc : npc2s) {
				npc->update(gameTime);
			}

			// collision detection
			// NPC1 hits other NPC1
			for (size_t i = 0; i < npcs.size(); i++)
			{
				for (size_t j = i + 1; j < npcs.size(); j++)
				{
					if (TUtils::intersects(
						npcs.at(i)->getDrawRectangle(),
						npcs.at(j)->getDrawRectangle()
						)) {

						npcs.at(i)->toggleStatus();
						npcs.at(j)->toggleStatus();
					}
				}
			}
			// NPC2 hits other NPC2
			for (size_t i = 0; i < npc2s.size(); i++)
			{
				for (size_t j = i + 1; j < npc2s.size(); j++)
				{
					if (TUtils::intersects(
						npc2s.at(i)->getDrawRectangle(),
						npc2s.at(j)->getDrawRectangle()
						)) {
						// If i is on the left, move i to the left
						// and j to the right
						if (npc2s.at(i)->getDrawRectangle().left <
							npc2s.at(j)->getDrawRectangle().left) {
							npc2s.at(i)->setStatus(1);
							npc2s.at(j)->setStatus(2);
						}
						else {
							npc2s.at(i)->setStatus(2);
							npc2s.at(j)->setStatus(1);
						}
					}
				}
			}

			// Collision between NPC1 and NPC2
			for (size_t i = 0; i < npcs.size(); i++) {
				for (size_t j = 0; j < npc2s.size(); j++) {
					if (npcs.at(i)->isActive() &&
						npc2s.at(j)->isActive() &&
						TUtils::intersects(npcs.at(i)->getDrawRectangle(), npc2s.at(j)->getDrawRectangle())) {

						// Mark both of them inactive
						npcs.at(i)->setActive(false);
						npc2s.at(j)->setActive(false);
					}
				}
			}

			// Remove inactive objects
			// Be careful here!!!
			// You must dereference the object first,
			// then remove the pointer from the vector.
			// If not..... memory leaks......
			// Don't blame me.
			for (auto it = npcs.begin(); it != npcs.end();) {
				if (!(*it)->isActive()) {
					delete * it;
					it = npcs.erase(it);
				}
				else {
					++it;
				}
			}
			for (auto it = npc2s.begin(); it != npc2s.end();) {
				if (!(*it)->isActive()) {
					delete * it;
					it = npc2s.erase(it);
				}
				else {
					++it;
				}
			}

			// Rest elapsed time to 0 and repaint screen
			gameTime.resetElapsedTime();
			InvalidateRect(mainWindowHandle, NULL, TRUE);
		}
		else {
			//
		}
		if (msg.message == WM_QUIT) break;
	}
}

HRESULT BaseApp::onRender()
{
	HRESULT hr = S_OK;

	renderingTarget->BeginDraw();

	renderingTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	renderingTarget->Clear(D2D1::ColorF(D2D1::ColorF::CornflowerBlue));

	// Draw background
	//background.draw(renderingTarget);

	for (auto npc : npcs) {
		if (npc->isActive())
			npc->draw(renderingTarget);
	}

	for (auto npc : npc2s) {
		if(npc->isActive()) 
			npc->draw(renderingTarget);
	}

	// Draw labels
	for (auto label : labels) {
		label.draw(renderingTarget);
	}

	renderingTarget->EndDraw();

	return hr;
}

void BaseApp::onResize(UINT width, UINT height)
{
	if (renderingTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.
		renderingTarget->Resize(D2D1::SizeU(width, height));
	}
}

HRESULT BaseApp::onKeyDown(WPARAM wparam) {
	HRESULT hr = S_OK;
	switch (wparam) {
	case VK_LEFT:
		break;
	case VK_RIGHT:
		break;
	case VK_UP:
		break;
	case VK_DOWN:
		break;
	}
	return hr;
}

void BaseApp::loadConfig() {
	// Need to load content from config before this methods.
	// This section will be handled by your future Config object.
	//

	// Background - spriteSheetFile.at(0)
	spriteSheetFiles.push_back("..//media//DubboITBackground.jpg");
	// Background - sprites.at(0)
	sprites.push_back({
		D2D1::RectF(0.f, 0.f, 799.f, 599.f)
	});

	// NPC 1 - spriteSheetFile.at(1)
	spriteSheetFiles.push_back("..//media//Regular_NPCs.png");
	// NPC 1 - sprites.at(1)
	sprites.push_back({
		D2D1::RectF(0.f, 0.f, 35.f, 60.f)
	});
	// NPC 2 - sprites.at(2)
	sprites.push_back({
		D2D1::RectF(251.f, 8.f, 282.f, 56.f)
	});

	// Fonts 
	fonts.push_back("Bookman Old Style");
	// Font sizes
	fontSizes.push_back(24.f);
	fontSizes.push_back(12.f);
	// Now that we are generating fonts and font sizes automatically,
	// the access to textFormats vector will be calculated by
	// (font * fontSize + fontSize)
	// i.e. textFormat.at(font * fontSize + fontSize)
	// Therefore, you should document the actual access index, here
	//
	// textFormat.at(0) : Bookman Old Style, 24.f
	// textFormat.at(1) : Bookman Old Style, 12.f

	brushColor.push_back(D2D1::ColorF(D2D1::ColorF::White));
	brushColor.push_back(D2D1::ColorF(D2D1::ColorF::DarkBlue));
	brushColor.push_back(D2D1::ColorF(D2D1::ColorF::Yellow));
	brushColor.push_back(D2D1::ColorF(D2D1::ColorF::Red));
}

void BaseApp::initializeGameAssets() {
	// This needs to be done after having devices created and config files loaded.

	// Set the background image
	background.setUp(spriteSheets.at(0), sprites.at(0));

	spawnNPC1();

	// Spawn NPC type 2
	spawnNPC2();
	
	// YOU: change the label to indicate the exercise name
	//
	// Labels
	labels.push_back(
		{solidBrushes.at(3), 560, 10, 790, 50, "DubboIT", textFormats.at(0)}
	);
	labels.push_back(
	{ solidBrushes.at(2), 560, 50, 790, 70, "by Your Name", textFormats.at(1) }
	);

}

// Mouse function
void BaseApp::onLButtonDown(int pixelX, int pixelY, DWORD flags)
{
	// Convert to proper location
	// ==========================
	// We only want to use the digits before the decimal point
	// TN
	D2D1_POINT_2F p = DPIScale::PixelsToDips(pixelX, pixelY);
	float xIntPart;
	float yIntPart;
	modf(p.x, &xIntPart);
	modf(p.y, &yIntPart);
	// Use the xIntPart and the yIntPart for your location
	
}

void BaseApp::onMouseMove(int pixelX, int pixelY, DWORD flags)
{
	// This is an example on how to handle the mousemove
	// TN
	if (flags & MK_LBUTTON)
	{
		const D2D1_POINT_2F dips = DPIScale::PixelsToDips(pixelX, pixelY);
	}
}

void BaseApp::onLButtonUp()
{
	// Handle the left button release
	// TN
}


void BaseApp::spawnNPC1() {
	while (npcs.size() < 5) {
		npcs.push_back(new NPC1());
		npcs.at(npcs.size() - 1)->setUp(spriteSheets.at(spriteSheets.size() - 1), sprites.at(1));
		npcs.at(npcs.size() - 1)->setLocation(getRandomPoint2D().x,getRandomPoint2D().y);
		npcs.at(npcs.size() - 1)->setPivotalPoint(getRandomPoint2D());
		npcs.at(npcs.size() - 1)->setRandomAngle();
		npcs.at(npcs.size() - 1)->setFrametoUpdateAngle(getRandomFrameToAngle());
		npcs.at(npcs.size() - 1)->setRadius(getRandomRadius());
	}
}

void BaseApp::spawnNPC2() {
	while (npc2s.size() < 5) {
		npc2s.push_back(new NPC2());
		npc2s.at(npc2s.size() - 1)->setUp(spriteSheets.at(spriteSheets.size() - 1), sprites.at(2));
		npc2s.at(npc2s.size() - 1)->setLocation(getRandomPoint2D().x, getRandomPoint2D().y);
	}
}

D2D1_POINT_2F BaseApp::getRandomPoint2D() {
	std::default_random_engine e(std::random_device{}());
	std::uniform_int_distribution<int> u(100, 600);
	std::uniform_int_distribution<int> t(100, 600);
	return D2D1::Point2F(static_cast<float>(u(e)), static_cast<float>(t(e)));
}

int BaseApp::getRandomFrameToAngle() {
	return TUtils::randIntBetween(3, 7);
}

float BaseApp::getRandomRadius() {
	return static_cast<float>(TUtils::randIntBetween(50,150));
}