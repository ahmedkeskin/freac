 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2006 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <dialogs/cddb/extsettings.h>
#include <resources.h>

BonkEnc::cddbExtendedSettingsDlg::cddbExtendedSettingsDlg(Int tab)
{
	currentConfig = BonkEnc::currentConfig;

	Point	 pos;
	Size	 size;

	mainWnd			= new Window(BonkEnc::i18n->TranslateString("Extended CDDB settings"), Point(140, 140), Size(350, 192));
	mainWnd_titlebar	= new Titlebar(TB_CLOSEBUTTON);
	divbar			= new Divider(42, OR_HORZ | OR_BOTTOM);

	register_layer_http	= new Layer(BonkEnc::i18n->TranslateString("HTTP settings"));
	register_layer_proxy	= new Layer(BonkEnc::i18n->TranslateString("Proxy settings"));

	pos.x = 175;
	pos.y = 29;
	size.cx = 0;
	size.cy = 0;

	btn_cancel		= new Button(BonkEnc::i18n->TranslateString("Cancel"), NIL, pos, size);
	btn_cancel->onAction.Connect(&cddbExtendedSettingsDlg::Cancel, this);
	btn_cancel->SetOrientation(OR_LOWERRIGHT);

	pos.x -= 88;

	btn_ok			= new Button(BonkEnc::i18n->TranslateString("OK"), NIL, pos, size);
	btn_ok->onAction.Connect(&cddbExtendedSettingsDlg::OK, this);
	btn_ok->SetOrientation(OR_LOWERRIGHT);

	pos.x = 7;
	pos.y = 7;
	size.cx = 329;
	size.cy = 106;

	reg_register		= new TabWidget(pos, size);

	pos.x = 7;
	pos.y = 11;
	size.cx = 312;
	size.cy = 66;

	http_group_scripts	= new GroupBox(BonkEnc::i18n->TranslateString("CGI scripts"), pos, size);

	pos.x = 16;
	pos.y = 24;

	http_text_query		= new Text(BonkEnc::i18n->TranslateString("CDDB query script:"), pos);

	pos.x += 101;
	pos.y -= 3;
	size.cx = 192;
	size.cy = 0;

	http_edit_query		= new EditBox(currentConfig->freedb_query_path, pos, size, 0);

	pos.x = 16;
	pos.y += 30;

	http_text_submit	= new Text(BonkEnc::i18n->TranslateString("CDDB submit script:"), pos);

	pos.x += 101;
	pos.y -= 3;

	http_edit_submit	= new EditBox(currentConfig->freedb_submit_path, pos, size, 0);

	Int	 maxTextSize = max(http_text_query->textSize.cx, http_text_submit->textSize.cx);

	http_edit_query->SetMetrics(Point(maxTextSize + 24, http_edit_query->GetY()), Size(285 - maxTextSize, http_edit_query->GetHeight()));
	http_edit_submit->SetMetrics(Point(maxTextSize + 24, http_edit_submit->GetX()), Size(285 - maxTextSize, http_edit_submit->GetHeight()));

	pos.x = 7;
	pos.y = 11;
	size.cx = 312;
	size.cy = 66;

	proxy_group_proxy	= new GroupBox(BonkEnc::i18n->TranslateString("Proxy settings"), pos, size);

	pos.x = 16;
	pos.y = 24;

	proxy_text_mode		= new Text(BonkEnc::i18n->TranslateString("Proxy type:"), pos);

	pos.x += 100;
	pos.y -= 3;
	size.cx = 185;
	size.cy = 0;

	proxy_combo_mode	= new ComboBox(pos, size);
	proxy_combo_mode->onSelectEntry.Connect(&cddbExtendedSettingsDlg::SetProxyMode, this);
	proxy_combo_mode->AddEntry(BonkEnc::i18n->TranslateString("no proxy"));
	proxy_combo_mode->AddEntry("HTTP");
	proxy_combo_mode->AddEntry("SOCKS4");
	proxy_combo_mode->AddEntry("SOCKS5");
	proxy_combo_mode->SelectNthEntry(currentConfig->freedb_proxy_mode);

	pos.x = 16;
	pos.y += 30;

	proxy_text_server	= new Text(BonkEnc::i18n->TranslateString("Proxy server:"), pos);

	pos.x += 100;
	pos.y -= 3;
	size.cx = 100;

	proxy_edit_server	= new EditBox(currentConfig->freedb_proxy, pos, size, 0);

	pos.x += 110;
	pos.y += 3;

	proxy_text_port		= new Text(BonkEnc::i18n->TranslateString("Port:"), pos);
	proxy_text_port->SetPosition(Point(264 - proxy_text_port->textSize.cx, proxy_text_port->GetY()));

	pos.x += 46;
	pos.y -= 3;
	size.cx = 37;

	proxy_edit_port		= new EditBox(String::FromInt(currentConfig->freedb_proxy_port), pos, size, 5);
	proxy_edit_port->SetFlags(EDB_NUMERIC);

	maxTextSize = max(proxy_text_mode->textSize.cx, proxy_text_server->textSize.cx);

	proxy_combo_mode->SetMetrics(Point(maxTextSize + 24, proxy_combo_mode->GetY()), Size(285 - maxTextSize, proxy_combo_mode->GetHeight()));
	proxy_edit_server->SetMetrics(Point(maxTextSize + 24, proxy_edit_server->GetY()), Size(233 - maxTextSize - proxy_text_port->textSize.cx, proxy_edit_server->GetHeight()));

	SetProxyMode();

	RegisterObject(mainWnd);

	mainWnd->RegisterObject(mainWnd_titlebar);
	mainWnd->RegisterObject(divbar);
	mainWnd->RegisterObject(btn_ok);
	mainWnd->RegisterObject(btn_cancel);
	mainWnd->RegisterObject(reg_register);

	reg_register->RegisterObject(register_layer_http);
	reg_register->RegisterObject(register_layer_proxy);

	register_layer_http->RegisterObject(http_group_scripts);
	register_layer_http->RegisterObject(http_text_query);
	register_layer_http->RegisterObject(http_edit_query);
	register_layer_http->RegisterObject(http_text_submit);
	register_layer_http->RegisterObject(http_edit_submit);

	register_layer_proxy->RegisterObject(proxy_group_proxy);
	register_layer_proxy->RegisterObject(proxy_text_mode);
	register_layer_proxy->RegisterObject(proxy_combo_mode);
	register_layer_proxy->RegisterObject(proxy_text_server);
	register_layer_proxy->RegisterObject(proxy_edit_server);
	register_layer_proxy->RegisterObject(proxy_text_port);
	register_layer_proxy->RegisterObject(proxy_edit_port);

	switch (tab)
	{
		case 0:
			reg_register->SelectTab(register_layer_http->GetHandle());
			break;
		case 1:
			reg_register->SelectTab(register_layer_proxy->GetHandle());
			break;
	}

	mainWnd->SetFlags(WF_NOTASKBUTTON);
	mainWnd->SetIcon(ImageLoader::Load("BonkEnc.pci:0"));
}

BonkEnc::cddbExtendedSettingsDlg::~cddbExtendedSettingsDlg()
{
	DeleteObject(mainWnd_titlebar);
	DeleteObject(mainWnd);
	DeleteObject(divbar);
	DeleteObject(reg_register);
	DeleteObject(register_layer_http);
	DeleteObject(register_layer_proxy);
	DeleteObject(http_group_scripts);
	DeleteObject(http_text_query);
	DeleteObject(http_edit_query);
	DeleteObject(http_text_submit);
	DeleteObject(http_edit_submit);
	DeleteObject(proxy_group_proxy);
	DeleteObject(proxy_text_mode);
	DeleteObject(proxy_combo_mode);
	DeleteObject(proxy_text_server);
	DeleteObject(proxy_edit_server);
	DeleteObject(proxy_text_port);
	DeleteObject(proxy_edit_port);
	DeleteObject(btn_ok);
	DeleteObject(btn_cancel);
}

const Error &BonkEnc::cddbExtendedSettingsDlg::ShowDialog()
{
	mainWnd->Stay();

	return error;
}

Void BonkEnc::cddbExtendedSettingsDlg::OK()
{
	currentConfig->freedb_query_path = http_edit_query->GetText();
	currentConfig->freedb_submit_path = http_edit_submit->GetText();

	currentConfig->freedb_proxy_mode = proxy_combo_mode->GetSelectedEntryNumber();
	currentConfig->freedb_proxy = proxy_edit_server->GetText();
	currentConfig->freedb_proxy_port = proxy_edit_port->GetText().ToInt();

	mainWnd->Close();
}

Void BonkEnc::cddbExtendedSettingsDlg::Cancel()
{
	mainWnd->Close();
}

Void BonkEnc::cddbExtendedSettingsDlg::SetProxyMode()
{
	if (proxy_combo_mode->GetSelectedEntryNumber() == 0)
	{
		proxy_text_server->Deactivate();
		proxy_edit_server->Deactivate();
		proxy_text_port->Deactivate();
		proxy_edit_port->Deactivate();
	}
	else
	{
		proxy_text_server->Activate();
		proxy_edit_server->Activate();
		proxy_text_port->Activate();
		proxy_edit_port->Activate();
	}

	if (proxy_combo_mode->GetSelectedEntryNumber() == 1 && proxy_edit_port->GetText().ToInt() == 1080)
	{
		proxy_edit_port->SetText("80");
	}

	if (proxy_combo_mode->GetSelectedEntryNumber() >= 2 && proxy_edit_port->GetText().ToInt() == 80)
	{
		proxy_edit_port->SetText("1080");
	}
}