/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

// Mac specific preference defaults
// TBD: Move Mac-specific Mime fields here?

platform.mac = true;

pref("browser.mac.show_tool_tips",              true);
pref("browser.mac.use_internet_config",         false);
pref("browser.mac.active_scrolling",            true);
pref("browser.mac.use_utility_pattern",         false);
pref("browser.mac.use_grayscale_form_controls", true);

pref("editor.use_html_editor",              false);
pref("editor.use_image_editor",             false);

//print header/footer
pref("browser.mac.print_header_topleft",    3);
pref("browser.mac.print_header_topmid",     5);
pref("browser.mac.print_header_topright",   2);
pref("browser.mac.print_header_botleft",    0);
pref("browser.mac.print_header_botmid",     4);
pref("browser.mac.print_header_botright",   0);
pref("browser.mac.print_background",        false); // checkbox

pref("mail.notification.sound",             "");
pref("mail.close_message_window.on_delete", true);
pref("mail.close_message_window.on_file", true);

pref("taskbar.mac.is_open",                 true);
pref("taskbar.mac.is_vertical",             true);

pref("mail.server_type_on_restart",         -1);


// This overrides the setting in config.js (which points to the Windows page)
config("menu.help.item_0.label","Help Contents");
config("menu.help.item_1.url","http://home.netscape.com/eng/mozilla/4.0/relnotes/mac-4.02.html");

mime_type("mime.image_gif", "image/gif", "gif", 2, "JPEGView", "JVWR", "GIFf");
mime_type("mime.image_jpeg", "image/jpeg", "jpg,jpeg,jpe", 2, "JPEGView", "JVWR", "JPEG");
mime_type("mime.image_pict", "image/pict", "pic,pict", 1, "SimpleText", "ttxt", "PICT");
mime_type("mime.image_tiff", "image/tiff", "tif,tiff", 1, "JPEGView", "JVWR", "TIFF");
mime_type("mime.image_x_xbitmap", "image/x-xbitmap", "xbm", 2, "Unknown", "ttxt", "????");

mime_type("mime.audio_basic", "audio/basic", "au,snd", 1, "Sound Machine", "SNDM", "ULAW");
mime_type("mime.audio_aiff", "audio/aiff", "aiff,aif", 1, "Sound Machine", "SNDM", "AIFF");
mime_type("mime.audio_x_wav", "audio/x-wav", "wav", 1, "Sound Machine", "SNDM", "WAVE");

mime_type("mime.video_quicktime", "video/quicktime", "qt,mov", 1, "Simple Player", "TVOD", "MooV");
mime_type("mime.video_mpeg", "video/mpeg", "mpg,mpeg,mpe", 1, "Sparkle", "mMPG", "MPEG");
mime_type("mime.video_x_msvideo", "video/x-msvideo", "avi", 1, "AVI to QT Utility", "AVIC", "????");
mime_type("mime.video_x_qtc", "video/x-qtc", "qtc", 1, "Conferencing Helper Application", "mtwh", ".qtc");

mime_type("mime.application_mac_binhex40", "application/mac-binhex40", "hqx", 1, "Stuffit Expander", "SITx", "TEXT");
mime_type("mime.application_x_stuffit", "application/x-stuffit", "sit", 1, "Stuffit Expander", "SITx", "SITD");
mime_type("mime.application_x_macbinary", "application/x-macbinary", "bin", 1, "Stuffit Expander", "SITx", "TEXT");
mime_type("mime.application_x_conference", "application/x-conference", "nsc", 1, "Netscape Conference", "Ncq�", "TEXT");

mime_type("mime.application_zip", "application/zip", "z,zip", 1, "ZipIt", "ZIP ", "ZIP ");
mime_type("mime.application_gzip", "application/gzip", "gz", 1, "MacGzip", "Gzip", "Gzip");
mime_type("mime.application_msword", "application/msword", "doc", 0, "MS Word", "MSWD", "W6BN");
mime_type("mime.application_x_excel", "application/x-excel", "xls", 0, "MS Excel", "XCEL", "XLS5");

mime_type("mime.application_octet_stream", "application/octet-stream", "exe", 3, "", "", "");
mime_type("mime.application_postscript", "application/postscript", "ai,eps,ps", 3, "SimpleText", "ttxt", "TEXT");
mime_type("mime.application_rtf", "application/rtf", "rtf", 0, "MS Word", "MSWD", "TEXT");
mime_type("mime.application_x_compressed", "application/x-compressed", ".Z", 1, "MacCompress", "LZIV", "ZIVM");
mime_type("mime.application_x_tar", "application/x-tar", "tar", 1, "tar", "TAR ", "TARF");

mime_type("mime.Netscape_Source", "Netscape/Source", "", 2, "SimpleText", "ttxt", "TEXT");
pref("mime.Netscape_Source.description", "View Source");
mime_type("mime.Netscape_Telnet", "Netscape/Telnet", "", 1, "NCSA Telnet", "NCSA", "CONF");
mime_type("mime.Netscape_tn3270", "Netscape/tn3270", "", 1, "tn3270", "GFTM", "GFTS");

mime_type("mime.image_x_cmu_raster", "image/x-cmu-raster", "ras", 3, "Unknown", "ttxt", "????");
mime_type("mime.image_x_portable_anymap", "image/x-portable-anymap", "pnm", 3, "Unknown", "ttxt", "????");
mime_type("mime.image_x_portable_bitmap", "image/x-portable-bitmap", "pbm", 3, "Unknown", "ttxt", "????");
mime_type("mime.image_x_portable_graymap", "image/x-portable-graymap", "pgm", 3, "Unknown", "ttxt", "????");
mime_type("mime.image_x_rgb", "image/x-rgb", "rgb", 3, "Unknown", "ttxt", "????");
mime_type("mime.image_x_xpixmap", "image/x-xpixmap", "xpm", 2, "Unknown", "ttxt", "????");

// Add mime type for SimpleText read only files
mime_type("mime.SimpleText_ReadOnly", "text/plain", "", 2, "SimpleText", "ttxt", "ttro");
