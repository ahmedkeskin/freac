 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2010 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#ifndef H_BONKENC
#define H_BONKENC

#include <smooth.h>
#include <boca.h>

#include "config.h"
#include "engine/converter.h"
#include "cddb/cddb.h"
#include "cddb/cddbinfo.h"

using namespace smooth;
using namespace smooth::GUI;
using namespace smooth::GUI::Dialogs;

namespace BonkEnc
{
	class JobList;
	class Converter;
};

typedef unsigned long  uint32;
typedef unsigned short uint16;
typedef unsigned char  uint8;

namespace BonkEnc
{
	abstract class BonkEnc : public GUI::Application
	{
		protected:
			/* Singleton class, therefore protected constructor/destructor
			 */
			static BonkEnc		*instance;

						 BonkEnc();
			virtual			~BonkEnc();

			GUI::Window		*mainWnd;
			Statusbar		*mainWnd_statusbar;

			Hyperlink		*hyperlink;

			Config			*currentConfig;

			Bool			 dontUpdateInfo;
			Bool			 overwriteAll;
		public:
			static String		 appName;
			static String		 appLongName;
			static String		 version;
			static String		 shortVersion;
			static String		 cddbVersion;
			static String		 cddbMode;
			static String		 website;
			static String		 updatePath;

			JobList			*joblist;

			Converter		*encoder;

			/* Returns an existing instance of BonkEnc
			 */
			static BonkEnc		*Get();
	};
};

#endif
