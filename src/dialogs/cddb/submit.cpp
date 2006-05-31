 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2006 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#include <dialogs/cddb/submit.h>
#include <resources.h>
#include <dllinterfaces.h>
#include <joblist.h>
#include <cddb.h>
#include <utilities.h>

typedef struct
{
	unsigned char	 packType;
	unsigned char	 trackNumber;
	unsigned char	 sequenceNumber;

	unsigned char	 characterPosition	:4;
	unsigned char	 block			:3;
	unsigned char	 bDBC			:1;

	unsigned char	 data[12];
	unsigned char	 crc0;
	unsigned char	 crc1;
}
cdTextPackage;

BonkEnc::cddbSubmitDlg::cddbSubmitDlg()
{
	currentConfig	= BonkEnc::currentConfig;

	activedrive	= currentConfig->cdrip_activedrive;
	updateJoblist	= currentConfig->update_joblist;

	dontUpdateInfo	= False;

	cddbInfo	= NIL;
	ownCddbInfo	= False;

	Point	 pos;
	Size	 size;

	mainWnd			= new Window(BonkEnc::i18n->TranslateString("CDDB data"), Point(120, 120), Size(500, 445));
	mainWnd->SetRightToLeft(BonkEnc::i18n->IsActiveLanguageRightToLeft());

	mainWnd_titlebar	= new Titlebar(TB_CLOSEBUTTON);
	divbar			= new Divider(42, OR_HORZ | OR_BOTTOM);

	pos.x = 175;
	pos.y = 29;
	size.cx = 0;
	size.cy = 0;

	btn_cancel		= new Button(BonkEnc::i18n->TranslateString("Cancel"), NIL, pos, size);
	btn_cancel->onAction.Connect(&cddbSubmitDlg::Cancel, this);
	btn_cancel->SetOrientation(OR_LOWERRIGHT);

	pos.x -= 88;

	btn_submit		= new Button(BonkEnc::i18n->TranslateString("Submit"), NIL, pos, size);
	btn_submit->onAction.Connect(&cddbSubmitDlg::Submit, this);
	btn_submit->SetOrientation(OR_LOWERRIGHT);

	pos.x = 7;
	pos.y = 27;

	check_updateJoblist	= new CheckBox(BonkEnc::i18n->TranslateString("Update joblist with this information"), pos, size, &updateJoblist);
	check_updateJoblist->SetWidth(check_updateJoblist->textSize.cx + 21);
	check_updateJoblist->SetOrientation(OR_LOWERLEFT);

	pos.x = 7;
	pos.y = 11;
	size.cx = 480;
	size.cy = 43;

	group_drive	= new GroupBox(BonkEnc::i18n->TranslateString("Active CD-ROM drive"), pos, size);

	pos.x = 17;
	pos.y = 23;
	size.cx = 260;
	size.cy = 0;

	combo_drive	= new ComboBox(pos, size);

	for (int j = 0; j < currentConfig->cdrip_numdrives; j++)
	{
		combo_drive->AddEntry(currentConfig->cdrip_drives.GetNthEntry(j));
	}

	combo_drive->SelectNthEntry(activedrive);
	combo_drive->onSelectEntry.Connect(&cddbSubmitDlg::ChangeDrive, this);

	pos.x += 267;
	pos.y += 3;

	text_cdstatus	= new Text(BonkEnc::i18n->TranslateString("Status").Append(":"), pos);

	pos.x = 7;
	pos.y = 65;

	text_artist	= new Text(BonkEnc::i18n->TranslateString("Artist").Append(":"), pos);

	pos.y += 27;

	text_album	= new Text(BonkEnc::i18n->TranslateString("Album").Append(":"), pos);

	pos.x += (7 + (Int) Math::Max(text_artist->textSize.cx, text_album->textSize.cx));
	pos.y -= 30;
	size.cx = 200 - (Int) Math::Max(text_artist->textSize.cx, text_album->textSize.cx);
	size.cy = 0;

	edit_artist	= new EditBox("", pos, size, 0);

	list_artist	= new List();
	list_artist->AddEntry(BonkEnc::i18n->TranslateString("Various artists"));

	edit_artist->SetDropDownList(list_artist);
	edit_artist->onInput.Connect(&cddbSubmitDlg::SetArtist, this);

	pos.y += 27;

	edit_album	= new EditBox("", pos, size, 0);

	list_genre	= new ListBox(pos, size);
	Utilities::FillGenreList(list_genre);

	pos.x = 221;
	pos.y = 65;

	text_year	= new Text(BonkEnc::i18n->TranslateString("Year").Append(":"), pos);

	pos.y += 27;

	text_disccomment= new Text(BonkEnc::i18n->TranslateString("Comment").Append(":"), pos);

	pos.x = 228 + Math::Max(text_year->textSize.cx, text_disccomment->textSize.cx);
	pos.y -= 30;
	size.cx = 31;

	edit_year	= new EditBox("", pos, size, 4);
	edit_year->SetFlags(EDB_NUMERIC);

	pos.x += 38;
	pos.y += 3;

	text_genre	= new Text(BonkEnc::i18n->TranslateString("Genre").Append(":"), pos);

	pos.x += (7 + text_genre->textSize.cx);
	pos.y -= 3;
	size.cx = 214 - text_genre->textSize.cx - Math::Max(text_year->textSize.cx, text_disccomment->textSize.cx);
	size.cy = 0;

	edit_genre	= new EditBox("", pos, size, 0);
	edit_genre->SetDropDownList(list_genre);

	pos.x = 228 + Math::Max(text_year->textSize.cx, text_disccomment->textSize.cx);
	pos.y += 27;
	size.cx = 259 - Math::Max(text_year->textSize.cx, text_disccomment->textSize.cx);
	size.cy = 34;

	edit_disccomment= new MultiEdit("", pos, size, 0);

	pos.x = 7;
	pos.y += 42;
	size.cx = 480;
	size.cy = 140;

	list_tracks	= new ListBox(pos, size);
	list_tracks->SetFlags(LF_ALLOWRESELECT);
	list_tracks->AddTab(BonkEnc::i18n->TranslateString("Track"), 50);
	list_tracks->AddTab(BonkEnc::i18n->TranslateString("Title"));
	list_tracks->onSelectEntry.Connect(&cddbSubmitDlg::SelectTrack, this);

	pos.x -= 1;
	pos.y += 151;

	text_track	= new Text(BonkEnc::i18n->TranslateString("Track").Append(":"), pos);

	pos.x += (7 + text_track->textSize.cx);
	pos.y -= 3;
	size.cx = 25;
	size.cy = 0;

	edit_track	= new EditBox("", pos, size, 3);
	edit_track->SetFlags(EDB_NUMERIC);
	edit_track->Deactivate();

	pos.x += 32;
	pos.y += 3;

	text_trackartist= new Text(BonkEnc::i18n->TranslateString("Artist").Append(":"), pos);

	pos.y += 27;

	text_title	= new Text(BonkEnc::i18n->TranslateString("Title").Append(":"), pos);

	pos.y += 27;

	text_comment	= new Text(BonkEnc::i18n->TranslateString("Comment").Append(":"), pos);

	pos.x += (7 + Math::Max(text_title->textSize.cx, text_comment->textSize.cx));
	pos.y -= 57;
	size.cx = 435 - Math::Max(text_title->textSize.cx, text_comment->textSize.cx) - text_track->textSize.cx;

	edit_trackartist= new EditBox("", pos, size, 0);
	edit_trackartist->onInput.Connect(&cddbSubmitDlg::UpdateTrack, this);
	edit_trackartist->onEnter.Connect(&cddbSubmitDlg::FinishArtist, this);

	pos.y += 27;

	edit_title	= new EditBox("", pos, size, 0);
	edit_title->onInput.Connect(&cddbSubmitDlg::UpdateTrack, this);
	edit_title->onEnter.Connect(&cddbSubmitDlg::FinishTrack, this);

	pos.y += 27;
	size.cy = 34;

	edit_comment	= new MultiEdit("", pos, size, 0);
	edit_comment->onInput.Connect(&cddbSubmitDlg::UpdateComment, this);

	pos.x = 6;
	pos.y = 25;

	text_status	= new Text("", pos);
	text_status->SetOrientation(OR_LOWERLEFT);

	SetArtist();

	RegisterObject(mainWnd);

	mainWnd->RegisterObject(btn_submit);
	mainWnd->RegisterObject(btn_cancel);
	mainWnd->RegisterObject(check_updateJoblist);
	mainWnd->RegisterObject(combo_drive);
	mainWnd->RegisterObject(group_drive);
	mainWnd->RegisterObject(text_artist);
	mainWnd->RegisterObject(edit_artist);
	mainWnd->RegisterObject(list_artist);
	mainWnd->RegisterObject(text_album);
	mainWnd->RegisterObject(edit_album);
	mainWnd->RegisterObject(text_genre);
	mainWnd->RegisterObject(edit_genre);
	mainWnd->RegisterObject(text_year);
	mainWnd->RegisterObject(edit_year);
	mainWnd->RegisterObject(text_disccomment);
	mainWnd->RegisterObject(edit_disccomment);
	mainWnd->RegisterObject(text_track);
	mainWnd->RegisterObject(edit_track);
	mainWnd->RegisterObject(text_trackartist);
	mainWnd->RegisterObject(edit_trackartist);
	mainWnd->RegisterObject(text_title);
	mainWnd->RegisterObject(edit_title);
	mainWnd->RegisterObject(text_comment);
	mainWnd->RegisterObject(edit_comment);
	mainWnd->RegisterObject(list_tracks);
	mainWnd->RegisterObject(text_cdstatus);
	mainWnd->RegisterObject(text_status);
	mainWnd->RegisterObject(mainWnd_titlebar);
	mainWnd->RegisterObject(divbar);

	mainWnd->SetFlags(mainWnd->GetFlags() | WF_NOTASKBUTTON);
	mainWnd->SetIcon(ImageLoader::Load("BonkEnc.pci:0"));
}

BonkEnc::cddbSubmitDlg::~cddbSubmitDlg()
{
	DeleteObject(mainWnd_titlebar);
	DeleteObject(mainWnd);
	DeleteObject(divbar);
	DeleteObject(combo_drive);
	DeleteObject(group_drive);
	DeleteObject(text_artist);
	DeleteObject(edit_artist);
	DeleteObject(list_artist);
	DeleteObject(text_album);
	DeleteObject(edit_album);
	DeleteObject(text_genre);
	DeleteObject(edit_genre);
	DeleteObject(list_genre);
	DeleteObject(text_year);
	DeleteObject(edit_year);
	DeleteObject(text_disccomment);
	DeleteObject(edit_disccomment);
	DeleteObject(list_tracks);
	DeleteObject(text_track);
	DeleteObject(edit_track);
	DeleteObject(text_trackartist);
	DeleteObject(edit_trackartist);
	DeleteObject(text_title);
	DeleteObject(edit_title);
	DeleteObject(text_comment);
	DeleteObject(edit_comment);
	DeleteObject(text_cdstatus);
	DeleteObject(text_status);
	DeleteObject(check_updateJoblist);
	DeleteObject(btn_submit);
	DeleteObject(btn_cancel);

	if (ownCddbInfo && cddbInfo != NIL) delete cddbInfo;
}

const Error &BonkEnc::cddbSubmitDlg::ShowDialog()
{
	ChangeDrive();

	mainWnd->Stay();

	return error;
}

Void BonkEnc::cddbSubmitDlg::Submit()
{
	Bool	 sane = True;

	if (edit_artist->GetText() == "")	sane = False;
	if (edit_album->GetText() == "")	sane = False;

	for (Int i = 0; i < titles.GetNOfEntries(); i++)
	{
		if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists") && artists.GetNthEntry(i) == "") sane = False;
		if (titles.GetNthEntry(i) == "") sane = False;
	}

	if (!sane)
	{
		Utilities::ErrorMessage("Please fill all fields and track titles before submitting.");

		return;
	}

	cddbInfo->dArtist	= (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists") ? "Various" : edit_artist->GetText());

	cddbInfo->dTitle	= edit_album->GetText();
	cddbInfo->dYear		= edit_year->GetText().ToInt();
	cddbInfo->dGenre	= edit_genre->GetText();
	cddbInfo->comment	= edit_disccomment->GetText();

	cddbInfo->trackArtists.RemoveAll();
	cddbInfo->trackTitles.RemoveAll();
	cddbInfo->trackComments.RemoveAll();

	for (Int j = 0; j < artists.GetNOfEntries(); j++) cddbInfo->trackArtists.AddEntry(artists.GetNthEntry(j));
	for (Int k = 0; k < titles.GetNOfEntries(); k++) cddbInfo->trackTitles.AddEntry((titles.GetNthEntry(k) == BonkEnc::i18n->TranslateString("Data track") ? "Data track" : titles.GetNthEntry(k)));
	for (Int l = 0; l < comments.GetNOfEntries(); l++) cddbInfo->trackComments.AddEntry(comments.GetNthEntry(l));

	if (cddbInfo->category == "") cddbInfo->category = GetCDDBGenre(edit_genre->GetText());

	cddbInfo->revision++;

	check_updateJoblist->Hide();
	text_status->SetText(BonkEnc::i18n->TranslateString("Submitting CD information").Append("..."));

	CDDB	 cddb(currentConfig);

	if (!cddb.Submit(cddbInfo))
	{
		Utilities::ErrorMessage("Some error occurred trying to connect to the freedb server.");

		text_status->SetText("");
		check_updateJoblist->Show();

		cddbInfo->revision--;

		return;
	}

	text_status->SetText("");

	if (updateJoblist)
	{
		for (Int l = 0; l < currentConfig->appMain->joblist->GetNOfTracks(); l++)
		{
			Track	*trackInfo = currentConfig->appMain->joblist->GetNthTrack(l);

			cddb.SetActiveDrive(activedrive);

			if (trackInfo->discid != cddb.DiscIDToString(cddb.ComputeDiscID())) continue;

			for (Int m = 0; m < titles.GetNOfEntries(); m++)
			{
				if (trackInfo->cdTrack == list_tracks->GetNthEntry(m)->GetText().ToInt())
				{
					if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists")) trackInfo->artist = artists.GetNthEntry(m);
					else										 trackInfo->artist = edit_artist->GetText();

					trackInfo->title	= titles.GetNthEntry(m);
					trackInfo->album	= edit_album->GetText();
					trackInfo->year		= edit_year->GetText().ToInt();
					trackInfo->genre	= edit_genre->GetText();
					trackInfo->comment	= comments.GetNthEntry(m);

					String	 jlEntry;

					if (trackInfo->artist == NIL && trackInfo->title == NIL)	jlEntry = String(trackInfo->origFilename).Append("\t");
					else								jlEntry = String(trackInfo->artist.Length() > 0 ? trackInfo->artist : BonkEnc::i18n->TranslateString("unknown artist")).Append(" - ").Append(trackInfo->title.Length() > 0 ? trackInfo->title : BonkEnc::i18n->TranslateString("unknown title")).Append("\t");

					jlEntry.Append(trackInfo->track > 0 ? (trackInfo->track < 10 ? String("0").Append(String::FromInt(trackInfo->track)) : String::FromInt(trackInfo->track)) : String("")).Append("\t").Append(trackInfo->lengthString).Append("\t").Append(trackInfo->fileSizeString);

					if (currentConfig->appMain->joblist->GetNthEntry(l)->GetText() != jlEntry) currentConfig->appMain->joblist->GetNthEntry(l)->SetText(jlEntry);
				}
			}
		}
	}

	currentConfig->update_joblist = updateJoblist;

	mainWnd->Close();
}

Void BonkEnc::cddbSubmitDlg::Cancel()
{
	mainWnd->Close();
}

Void BonkEnc::cddbSubmitDlg::SetArtist()
{
	if (dontUpdateInfo) return;

	if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists") && !edit_trackartist->IsActive())
	{
		edit_trackartist->Activate();

		UpdateTrackList();

		if (list_tracks->GetSelectedEntry() != NIL) edit_trackartist->SetText(artists.GetEntry(list_tracks->GetSelectedEntry()->GetHandle()));
	}
	else if (edit_artist->GetText() != BonkEnc::i18n->TranslateString("Various artists") && edit_trackartist->IsActive())
	{
		edit_trackartist->SetText("");
		edit_trackartist->Deactivate();

		UpdateTrackList();
	}
}

Void BonkEnc::cddbSubmitDlg::UpdateTrackList()
{
	for (Int i = 0; i < list_tracks->GetNOfEntries(); i++)
	{
		if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists")) list_tracks->GetNthEntry(i)->SetText(String(i < 9 ? "0" : "").Append(String::FromInt(i + 1)).Append("\t").Append(artists.GetNthEntry(i).Length() > 0 ? artists.GetNthEntry(i) : BonkEnc::i18n->TranslateString("unknown artist")).Append(" - ").Append(titles.GetNthEntry(i).Length() > 0 ? titles.GetNthEntry(i) : BonkEnc::i18n->TranslateString("unknown title")));
		else										 list_tracks->GetNthEntry(i)->SetText(String(i < 9 ? "0" : "").Append(String::FromInt(i + 1)).Append("\t").Append(titles.GetNthEntry(i).Length() > 0 ? titles.GetNthEntry(i) : BonkEnc::i18n->TranslateString("unknown title")));
	}
}

Void BonkEnc::cddbSubmitDlg::ChangeDrive()
{
	if (ownCddbInfo && cddbInfo != NIL) delete cddbInfo;

	activedrive = combo_drive->GetSelectedEntryNumber();

	ex_CR_SetActiveCDROM(activedrive);
	ex_CR_ReadToc();

	Int	 numTocEntries = ex_CR_GetNumTocEntries();
	Int	 numAudioTracks = numTocEntries;

	for (Int i = 0; i < numTocEntries; i++)
	{
		TOCENTRY entry = ex_CR_GetTocEntry(i);

		if (entry.btFlag & CDROMDATAFLAG || entry.btTrackNumber != i + 1) numAudioTracks--;
	}

	if (numAudioTracks <= 0)
	{
		text_cdstatus->SetText(BonkEnc::i18n->TranslateString("Status").Append(": ").Append(BonkEnc::i18n->TranslateString("No audio CD in drive!")));

		dontUpdateInfo = True;

		edit_artist->SetText("");
		edit_album->SetText("");
		edit_year->SetText("");
		edit_genre->SetText("");
		edit_disccomment->SetText("");

		list_tracks->RemoveAllEntries();
		titles.RemoveAll();
		comments.RemoveAll();

		edit_track->SetText("");
		edit_trackartist->SetText("");
		edit_title->SetText("");
		edit_comment->SetText("");

		edit_trackartist->Deactivate();

		dontUpdateInfo = False;

		btn_submit->Deactivate();

		return;
	}
	else
	{
		text_cdstatus->SetText(BonkEnc::i18n->TranslateString("Status").Append(": ").Append(BonkEnc::i18n->TranslateString("Successfully read CD!")));

		btn_submit->Activate();
	}

	ReadCDText();

	Int	 oDrive = currentConfig->cdrip_activedrive;

	currentConfig->cdrip_activedrive = activedrive;

	CDDB		 cddb(currentConfig);
	Int		 iDiscid = cddb.ComputeDiscID();
	CDDBInfo	*cdInfo = NIL;

	if (currentConfig->enable_cddb_cache) cdInfo = CDDB::infoCache.GetEntry(iDiscid);

	if (cdInfo == NIL)
	{
		cdInfo = currentConfig->appMain->GetCDDBData();

		if (cdInfo != NIL)
		{
			CDDB::infoCache.RemoveEntry(iDiscid);
			CDDB::infoCache.AddEntry(cdInfo, iDiscid);
		}
	}

	currentConfig->cdrip_activedrive = oDrive;

	dontUpdateInfo = True;

	if (cdInfo != NIL)
	{
		if (cdInfo->dArtist == "Various") edit_artist->SetText(BonkEnc::i18n->TranslateString("Various artists"));
		else				  edit_artist->SetText(cdInfo->dArtist);

		edit_album->SetText(cdInfo->dTitle);
		edit_year->SetText(String::FromInt(cdInfo->dYear) == "0" ? String("") : String::FromInt(cdInfo->dYear));
		edit_genre->SetText(cdInfo->dGenre);
		edit_disccomment->SetText(cdInfo->comment);

		list_tracks->RemoveAllEntries();

		edit_track->SetText("");
		edit_trackartist->SetText("");
		edit_title->SetText("");
		edit_comment->SetText("");

		if (cdInfo->dArtist == "Various") edit_trackartist->Activate();
		else				  edit_trackartist->Deactivate();

		for (Int j = 0; j < cdInfo->trackTitles.GetNOfEntries(); j++)
		{
			Int	 handle = list_tracks->AddEntry("")->GetHandle();

			artists.AddEntry(cdInfo->trackArtists.GetNthEntry(j), handle);
			titles.AddEntry(cdInfo->trackTitles.GetNthEntry(j), handle);
			comments.AddEntry(cdInfo->trackComments.GetNthEntry(j), handle);
		}

		cddbInfo	= cdInfo;
		ownCddbInfo	= False;
	}
	else if (cdText.GetEntry(0) != NIL)
	{
		if (cdText.GetEntry(0) == "Various") edit_artist->SetText(BonkEnc::i18n->TranslateString("Various artists"));
		else				     edit_artist->SetText(cdText.GetEntry(0));

		edit_album->SetText(cdText.GetEntry(100));
		edit_year->SetText("");
		edit_genre->SetText("");
		edit_disccomment->SetText("");

		list_tracks->RemoveAllEntries();

		edit_track->SetText("");
		edit_trackartist->SetText("");
		edit_title->SetText("");
		edit_comment->SetText("");

		if (cdText.GetEntry(0) == "Various") edit_trackartist->Activate();
		else				     edit_trackartist->Deactivate();

		cddbInfo	= new CDDBInfo();
		ownCddbInfo	= True;

		cddbInfo->discID = iDiscid;
		cddbInfo->revision = -1;

		cddbInfo->dArtist = cdText.GetEntry(0);
		cddbInfo->dTitle = cdText.GetEntry(100);

		for (Int j = 0; j < numTocEntries; j++)
		{
			TOCENTRY entry = ex_CR_GetTocEntry(j);

			cddbInfo->trackOffsets.AddEntry(entry.dwStartSector + 150, j);
			cddbInfo->trackArtists.AddEntry("", j);
			cddbInfo->trackTitles.AddEntry(cdText.GetEntry(entry.btTrackNumber), j);
			cddbInfo->trackComments.AddEntry("", j);

			if (entry.btFlag & CDROMDATAFLAG)
			{
				Int	 handle = list_tracks->AddEntry("")->GetHandle();

				artists.AddEntry("", handle);
				titles.AddEntry(BonkEnc::i18n->TranslateString("Data track"), handle);
				comments.AddEntry("", handle);
			}
			else
			{
				Int	 handle = list_tracks->AddEntry("")->GetHandle();

				artists.AddEntry("", handle);
				titles.AddEntry(cdText.GetEntry(entry.btTrackNumber), handle);
				comments.AddEntry("", handle);
			}
		}

		cddbInfo->discLength = ex_CR_GetTocEntry(numTocEntries).dwStartSector / 75 - ex_CR_GetTocEntry(0).dwStartSector / 75 + 2;
	}
	else
	{
		edit_artist->SetText("");
		edit_album->SetText("");
		edit_year->SetText("");
		edit_genre->SetText("");
		edit_disccomment->SetText("");

		list_tracks->RemoveAllEntries();

		edit_track->SetText("");
		edit_trackartist->SetText("");
		edit_title->SetText("");
		edit_comment->SetText("");

		edit_trackartist->Deactivate();

		cddbInfo	= new CDDBInfo();
		ownCddbInfo	= True;

		cddbInfo->discID = iDiscid;
		cddbInfo->revision = -1;

		for (Int j = 0; j < numTocEntries; j++)
		{
			TOCENTRY entry = ex_CR_GetTocEntry(j);

			cddbInfo->trackOffsets.AddEntry(entry.dwStartSector + 150, j);
			cddbInfo->trackArtists.AddEntry("", j);
			cddbInfo->trackTitles.AddEntry("", j);
			cddbInfo->trackComments.AddEntry("", j);

			if (entry.btFlag & CDROMDATAFLAG)
			{
				Int	 handle = list_tracks->AddEntry("")->GetHandle();

				artists.AddEntry("", handle);
				titles.AddEntry(BonkEnc::i18n->TranslateString("Data track"), handle);
				comments.AddEntry("", handle);
			}
			else
			{
				Int	 handle = list_tracks->AddEntry("")->GetHandle();

				artists.AddEntry("", handle);
				titles.AddEntry("", handle);
				comments.AddEntry("", handle);
			}
		}

		cddbInfo->discLength = ex_CR_GetTocEntry(numTocEntries).dwStartSector / 75 - ex_CR_GetTocEntry(0).dwStartSector / 75 + 2;
	}

	for (Int l = 0; l < currentConfig->appMain->joblist->GetNOfTracks(); l++)
	{
		Track	*trackInfo = currentConfig->appMain->joblist->GetNthTrack(l);

		if (trackInfo->discid != cddb.DiscIDToString(cddb.ComputeDiscID())) continue;

		for (Int m = 0; m < titles.GetNOfEntries(); m++)
		{
			if (trackInfo->track == list_tracks->GetNthEntry(m)->GetText().ToInt() && trackInfo->artist != "" && trackInfo->artist != artists.GetNthEntry(m))
			{
				artists.SetEntry(artists.GetNthEntryIndex(m), trackInfo->artist);
				cddbInfo->trackArtists.SetEntry(cddbInfo->trackArtists.GetNthEntryIndex(trackInfo->cdTrack - 1), trackInfo->artist);
			}

			if (trackInfo->track == list_tracks->GetNthEntry(m)->GetText().ToInt() && trackInfo->title != "" && trackInfo->title != titles.GetNthEntry(m))
			{
				titles.SetEntry(titles.GetNthEntryIndex(m), trackInfo->title);
				cddbInfo->trackTitles.SetEntry(cddbInfo->trackTitles.GetNthEntryIndex(trackInfo->cdTrack - 1), trackInfo->title);
			}

			if (trackInfo->track == list_tracks->GetNthEntry(m)->GetText().ToInt() && trackInfo->comment != "" && trackInfo->comment != comments.GetNthEntry(m))
			{
				comments.SetEntry(comments.GetNthEntryIndex(m), trackInfo->comment);
				cddbInfo->trackComments.SetEntry(cddbInfo->trackComments.GetNthEntryIndex(trackInfo->cdTrack - 1), trackInfo->comment);
			}
		}
	}

	FreeCDText();

	UpdateTrackList();

	dontUpdateInfo = False;
}

Int BonkEnc::cddbSubmitDlg::ReadCDText()
{
	FreeCDText();

	const int	 nBufferSize	= 4 + 8 * sizeof(cdTextPackage) * 256;
	unsigned char	*pbtBuffer	= new unsigned char [nBufferSize];
	int		 nCDTextSize	= 0;
	char		*lpZero		= NIL;

	ex_CR_ReadCDText(pbtBuffer, nBufferSize, &nCDTextSize);

	if (nCDTextSize < 4) { delete [] pbtBuffer; return Error(); }

	int		 nNumPacks		= (nCDTextSize - 4) / sizeof(cdTextPackage);
	cdTextPackage	*pCDtextPacks		= NIL;
	char		 lpszBuffer[1024]	= {'\0',};
	int		 nInsertPos		= 0;

	for (Int i = 0; i < nNumPacks; i++)
	{
		pCDtextPacks = (cdTextPackage *) &pbtBuffer[i * sizeof(cdTextPackage) + 4];

		if (pCDtextPacks->block == 0)
		{
			for (Int j = 0; j < 12; j++) lpszBuffer[nInsertPos++] = pCDtextPacks->data[j];

			while (nInsertPos > 0 && (lpZero = (char *) memchr(lpszBuffer, '\0', nInsertPos)) != NIL)
			{
				Int	 nOut = (lpZero - lpszBuffer) + 1;

				if (pCDtextPacks->packType == 0x80) // Album/Track title
				{
					if (pCDtextPacks->trackNumber == 0) cdText.AddEntry(lpszBuffer, 100);
					if (pCDtextPacks->trackNumber != 0) cdText.AddEntry(lpszBuffer, pCDtextPacks->trackNumber);
				}
				else if (pCDtextPacks->packType == 0x81) // Artist name
				{
					if (pCDtextPacks->trackNumber == 0) cdText.AddEntry(lpszBuffer, 0);
				}

				nInsertPos -= nOut;

				memmove(lpszBuffer, lpZero + 1, 1024 - nOut -1);

				pCDtextPacks->trackNumber++;

				while (nInsertPos > 0 && lpszBuffer[ 0 ] == '\0')
				{
					memmove(lpszBuffer, lpszBuffer + 1, 1024 -1);
					nInsertPos--;
				}
			}
		}
	}

	delete [] pbtBuffer;

	return Success();
}

Int BonkEnc::cddbSubmitDlg::FreeCDText()
{
	cdText.RemoveAll();

	return Success();
}

Void BonkEnc::cddbSubmitDlg::SelectTrack()
{
	if (list_tracks->GetSelectedEntry() == NIL) return;

	String	 artist = artists.GetEntry(list_tracks->GetSelectedEntry()->GetHandle());
	String	 title = titles.GetEntry(list_tracks->GetSelectedEntry()->GetHandle());
	String	 comment = comments.GetEntry(list_tracks->GetSelectedEntry()->GetHandle());
	Int	 track = list_tracks->GetSelectedEntry()->GetText().ToInt();

	dontUpdateInfo = True;

	if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists")) edit_trackartist->SetText(artist);

	edit_title->SetText(title);
	edit_comment->SetText(comment);
	edit_track->SetText("");

	if (track > 0 && track < 10)	edit_track->SetText(String("0").Append(String::FromInt(track)));
	else if (track >= 10)		edit_track->SetText(String::FromInt(track));

	if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists")) edit_trackartist->MarkAll();
	else										 edit_title->MarkAll();

	dontUpdateInfo = False;
}

Void BonkEnc::cddbSubmitDlg::UpdateTrack()
{
	if (dontUpdateInfo) return;
	if (list_tracks->GetSelectedEntry() == NIL) return;

	Int	 track = edit_track->GetText().ToInt();

	if (edit_artist->GetText() == BonkEnc::i18n->TranslateString("Various artists")) list_tracks->GetSelectedEntry()->SetText(String(track < 10 ? "0" : "").Append(String::FromInt(track)).Append("\t").Append(edit_trackartist->GetText() == "" ? BonkEnc::i18n->TranslateString("unknown artist") : edit_trackartist->GetText()).Append(" - ").Append(edit_title->GetText() == "" ? BonkEnc::i18n->TranslateString("unknown title") : edit_title->GetText()));
	else										 list_tracks->GetSelectedEntry()->SetText(String(track < 10 ? "0" : "").Append(String::FromInt(track)).Append("\t").Append(edit_title->GetText() == "" ? BonkEnc::i18n->TranslateString("unknown title") : edit_title->GetText()));

	artists.SetEntry(list_tracks->GetSelectedEntry()->GetHandle(), edit_trackartist->GetText());
	titles.SetEntry(list_tracks->GetSelectedEntry()->GetHandle(), edit_title->GetText());
}

Void BonkEnc::cddbSubmitDlg::FinishArtist()
{
	edit_title->MarkAll();
}

Void BonkEnc::cddbSubmitDlg::FinishTrack()
{
	for (Int i = 0; i < list_tracks->GetNOfEntries() - 1; i++)
	{
		if (list_tracks->GetSelectedEntry() == list_tracks->GetNthEntry(i))
		{
			list_tracks->SelectEntry(list_tracks->GetNthEntry(i + 1));

			SelectTrack();

			break;
		}
	}
}

Void BonkEnc::cddbSubmitDlg::UpdateComment()
{
	if (dontUpdateInfo) return;
	if (list_tracks->GetSelectedEntry() == NIL) return;

	comments.SetEntry(list_tracks->GetSelectedEntry()->GetHandle(), edit_comment->GetText());
}

String BonkEnc::cddbSubmitDlg::GetCDDBGenre(const String &genre)
{
	String	 cddbGenre = "misc";

	if (genre == "Alt. Rock")		cddbGenre = "rock";
	if (genre == "Anime")			cddbGenre = "soundtrack";
	if (genre == "Big Band")		cddbGenre = "jazz";
	if (genre == "Black Metal")		cddbGenre = "rock";
	if (genre == "Blues")			cddbGenre = "blues";
	if (genre == "BritPop")			cddbGenre = "rock";
	if (genre == "Celtic")			cddbGenre = "folk";
	if (genre == "Chamber Music")		cddbGenre = "classical";
	if (genre == "Christian Rock")		cddbGenre = "rock";
	if (genre == "Classic Rock")		cddbGenre = "rock";
	if (genre == "Classical")		cddbGenre = "classical";
	if (genre == "Country")			cddbGenre = "country";
	if (genre == "Death Metal")		cddbGenre = "rock";
	if (genre == "Ethnic")			cddbGenre = "folk";
	if (genre == "Folk")			cddbGenre = "folk";
	if (genre == "Folk/Rock")		cddbGenre = "folk";
	if (genre == "Folklore")		cddbGenre = "folk";
	if (genre == "Gothic Rock")		cddbGenre = "rock";
	if (genre == "Hard Rock")		cddbGenre = "rock";
	if (genre == "Heavy Metal")		cddbGenre = "rock";
	if (genre == "Instrumental Pop")	cddbGenre = "rock";
	if (genre == "Instrumental Rock")	cddbGenre = "rock";
	if (genre == "Jazz")			cddbGenre = "jazz";
	if (genre == "Jazz+Funk")		cddbGenre = "jazz";
	if (genre == "JPop")			cddbGenre = "rock";
	if (genre == "Metal")			cddbGenre = "rock";
	if (genre == "National Folk")		cddbGenre = "folk";
	if (genre == "Native American")		cddbGenre = "folk";
	if (genre == "New Age")			cddbGenre = "newage";
	if (genre == "Pop")			cddbGenre = "rock";
	if (genre == "Pop/Funk")		cddbGenre = "rock";
	if (genre == "Pop-Folk")		cddbGenre = "folk";
	if (genre == "Progressive Rock")	cddbGenre = "rock";
	if (genre == "Psychedelic Rock")	cddbGenre = "rock";
	if (genre == "Punk")			cddbGenre = "rock";
	if (genre == "Punk Rock")		cddbGenre = "rock";
	if (genre == "Reggae")			cddbGenre = "reggae";
	if (genre == "Rock")			cddbGenre = "rock";
	if (genre == "Rock & Roll")		cddbGenre = "rock";
	if (genre == "Slow Rock")		cddbGenre = "rock";
	if (genre == "Soundtrack")		cddbGenre = "soundtrack";
	if (genre == "Southern Rock")		cddbGenre = "rock";
	if (genre == "Symphonic Rock")		cddbGenre = "rock";
	if (genre == "Symphony")		cddbGenre = "classical";
	if (genre == "Thrash-Metal")		cddbGenre = "rock";
	if (genre == "Top 40")			cddbGenre = "rock";
	if (genre == "Tribal")			cddbGenre = "folk";

	return cddbGenre;
}
