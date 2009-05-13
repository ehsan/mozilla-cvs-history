#!/usr/bin/ruby
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is generate_macosx_symbols.rb.
#
# The Initial Developer of the Original Code is
# Smokey Ardisson <alqahira@ardisson.org>.
# Portions created by the Initial Developer are Copyright (C) 2009
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Smokey Ardisson <alqahira@ardisson.org> (Original Author)
#   Stuart Morgan <stuart.morgan@alumni.case.edu>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

require 'fileutils'
require 'open3'

if ARGV.length != 3
  puts "Usage: $0 <symbolstore.py path> <dump_syms path> <output directory>"
  exit 1
end
(symbolstore_py_path, dump_syms_path, output_dir) = ARGV

os_version = `sw_vers -productVersion`.chomp.gsub('.', '_')
symbol_output_dir = File.join(output_dir, "macosx-#{os_version}-symbols")
symbol_archive_basename = "Mac_OS_X-#{os_version}"
symbol_list_file = File.join(symbol_output_dir, "#{symbol_archive_basename}-symbols.txt")
zip_archive_file = "crashreporter-symbols-#{symbol_archive_basename}.zip"

# This list of files is generated based on mxr results for "-framework" in 
# mozilla/, frameworks Camino links against, common hits in Camino breakpad
# reports, as well as a few cherry-picked forward-looking files (CoreText).
# AE and LaunchServices switched frameworks in Mac OS X 10.5, and 
# FindByContent and WebServicesCore have no binary on 10.5.
macosx_10_4_os_files = <<EOF
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/AE.framework/AE
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/LaunchServices.framework/LaunchServices
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/FindByContent.framework/FindByContent
/System/Library/Frameworks/CoreServices.framework/Frameworks/WebServicesCore.framework/WebServicesCore
EOF
  
macosx_10_5_os_files = <<EOF 
/System/Library/Frameworks/CoreServices.framework/Frameworks/AE.framework/AE
/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/LaunchServices
EOF
  
common_os_files = <<EOF
/System/Library/Frameworks/AddressBook.framework/AddressBook
/System/Library/Frameworks/AGL.framework/AGL
/System/Library/Frameworks/AppKit.framework/AppKit
/System/Library/Frameworks/AppKit.framework/Resources/BridgeSupport/AppKit.dylib
/System/Library/Frameworks/ApplicationServices.framework/ApplicationServices
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/ATS.framework/ATS
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/ATS.framework/Resources/*.dylib
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/ColorSync.framework/ColorSync
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/CoreGraphics
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Resources/*.dylib
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreGraphics.framework/Resources/BridgeSupport/CoreGraphics.dylib
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/CoreText.framework/CoreText
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/HIServices.framework/HIServices
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/ImageIO.framework/ImageIO
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/ImageIO.framework/Resources/*.dylib
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/LangAnalysis.framework/LangAnalysis
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/PrintCore.framework/PrintCore
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/QD.framework/QD
/System/Library/Frameworks/ApplicationServices.framework/Frameworks/SpeechSynthesis.framework/SpeechSynthesis
/System/Library/Frameworks/Carbon.framework/Carbon
/System/Library/Frameworks/Carbon.framework/Frameworks/CarbonSound.framework/CarbonSound
/System/Library/Frameworks/Carbon.framework/Frameworks/Help.framework/Help
/System/Library/Frameworks/Carbon.framework/Frameworks/CommonPanels.framework/CommonPanels
/System/Library/Frameworks/Carbon.framework/Frameworks/HIToolbox.framework/HIToolbox
/System/Library/Frameworks/Carbon.framework/Frameworks/HTMLRendering.framework/HTMLRendering
/System/Library/Frameworks/Carbon.framework/Frameworks/OpenScripting.framework/OpenScripting
/System/Library/Frameworks/Carbon.framework/Frameworks/OpenScripting.framework/Resources/*.dylib
/System/Library/Frameworks/Carbon.framework/Frameworks/ImageCapture.framework/ImageCapture
/System/Library/Frameworks/Carbon.framework/Frameworks/Ink.framework/Ink
/System/Library/Frameworks/Carbon.framework/Frameworks/NavigationServices.framework/NavigationServices
/System/Library/Frameworks/Carbon.framework/Frameworks/Print.framework/Print
/System/Library/Frameworks/Carbon.framework/Frameworks/SecurityHI.framework/SecurityHI
/System/Library/Frameworks/Carbon.framework/Frameworks/SpeechRecognition.framework/SpeechRecognition
/System/Library/Frameworks/Cocoa.framework/Cocoa
/System/Library/Frameworks/CoreAudio.framework/CoreAudio
/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation
/System/Library/Frameworks/CoreFoundation.framework/Resources/BridgeSupport/CoreFoundation.dylib
/System/Library/Frameworks/CoreServices.framework/CoreServices
/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/CarbonCore
/System/Library/Frameworks/CoreServices.framework/Frameworks/CFNetwork.framework/CFNetwork
/System/Library/Frameworks/CoreServices.framework/Frameworks/DictionaryServices.framework/DictionaryServices
/System/Library/Frameworks/CoreServices.framework/Frameworks/Metadata.framework/Metadata
/System/Library/Frameworks/CoreServices.framework/Frameworks/OSServices.framework/OSServices
/System/Library/Frameworks/CoreServices.framework/Frameworks/SearchKit.framework/SearchKit
/System/Library/Frameworks/ExceptionHandling.framework/ExceptionHandling
/System/Library/Frameworks/Foundation.framework/Foundation
/System/Library/Frameworks/Foundation.framework/Resources/BridgeSupport/Foundation.dylib
/System/Library/Frameworks/IOKit.framework/IOKit
/System/Library/Frameworks/JavaEmbedding.framework/JavaEmbedding
/System/Library/Frameworks/JavaVM.framework/JavaVM
/System/Library/Frameworks/JavaVM.framework/Frameworks/JavaNativeFoundation.framework/JavaNativeFoundation
/System/Library/Frameworks/JavaVM.framework/Versions/1.4.2/Libraries/*.dylib
/System/Library/Frameworks/JavaVM.framework/Versions/1.4.2/Resources/JavaPluginCocoa.bundle/Contents/MacOS/JavaPluginCocoa
/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Libraries/*.dylib
/System/Library/Frameworks/JavaVM.framework/Versions/1.5.0/Resources/JavaPluginCocoa.bundle/Contents/MacOS/JavaPluginCocoa
/System/Library/Frameworks/OpenGL.framework/OpenGL
/System/Library/Frameworks/OpenGL.framework/Libraries/*.dylib
/System/Library/Frameworks/PreferencePanes.framework/PreferencePanes
/System/Library/Frameworks/Python.framework/Python
/System/Library/Frameworks/Quartz.framework/Quartz
/System/Library/Frameworks/QuartzCore.framework/QuartzCore
/System/Library/Frameworks/QuickTime.framework/QuickTime
/System/Library/Frameworks/Security.framework/Security
/System/Library/Frameworks/Security.framework/PlugIns/csparser.bundle/Contents/MacOS/csparser
/System/Library/Frameworks/System.framework/System
/System/Library/Frameworks/SystemConfiguration.framework/SystemConfiguration
/System/Library/Input Methods/KoreanIM.app/Contents/HanjaTool.app/Contents/MacOS/HanjaTool
/System/Library/Input Methods/KoreanIM.app/Contents/MacOS/KoreanIM
/System/Library/Input Methods/Kotoeri.app/Contents/MacOS/Kotoeri
/System/Library/Input Methods/SCIM.app/Contents/ITABCEngine.framework/ITABCEngine
/System/Library/Input Methods/SCIM.app/Contents/MacOS/SCIM
/System/Library/Input Methods/SCIM.app/Contents/SCIMTool.app/Contents/MacOS/SCIMTool
/System/Library/Input Methods/TamilIM.app/Contents/MacOS/TamilIM
/System/Library/Input Methods/TCIM.app/Contents/MacOS/TCIM
/System/Library/Input Methods/TCIM.app/Contents/TCIMTool.app/Contents/MacOS/TCIMTool
/System/Library/Input Methods/VietnameseIM.app/Contents/MacOS/VietnameseIM
/System/Library/PrivateFrameworks/DesktopServicesPriv.framework/DesktopServicesPriv
/System/Library/QuickTime/QuickTimeComponents.component/Contents/MacOS/QuickTimeComponents
/usr/lib/libgcc_s.1.dylib
/usr/lib/libobjc.A.dylib
/usr/lib/libstdc++.6.dylib
/usr/lib/libSystem.B.dylib
/Library/Internet Plug-Ins/JavaPluginCocoa.bundle/Contents/MacOS/JavaPluginCocoa
/Library/Internet Plug-Ins/QuickTime Plugin.plugin/Contents/MacOS/QuickTime Plugin
/Library/Internet Plug-Ins/Flash Player.plugin/Contents/MacOS/Flash Player
/Library/Internet Plug-Ins/RealPlayer Plugin.plugin/Contents/MacOS/RealPlayer Plugin
EOF

symbol_files = common_os_files.split("\n")
if os_version.match(/^10_4/)
  symbol_files += macosx_10_4_os_files.split("\n")
elsif os_version.match(/^10_5/)
  symbol_files += macosx_10_5_os_files.split("\n")
end

# These won't be going through a shell, so take care of globbing ourselves.
symbol_files.collect! { |path| Dir.glob(path) }.flatten!

puts "Breakpad symbols for Mac OS X #{os_version}"

puts "Preparing symbol storage directory"
FileUtils.mkdir_p symbol_output_dir

puts "\nGenerating OS symbols"
Open3.popen3("python", symbolstore_py_path, "-a", "ppc i386", dump_syms_path, symbol_output_dir, *symbol_files) do |stdin, stdout, stderr| 
  File.open(symbol_list_file, "w") { |file| file.write(stdout.read) }
  puts stderr.read
end

puts "\nZipping symbols"
system("cd '#{symbol_output_dir}' && zip -r9D '../#{zip_archive_file}' .")

puts "\nMac OS X #{os_version} symbols generated in #{symbol_output_dir}"
puts "Mac OS X #{os_version} symbol archive generated in #{output_dir}/#{zip_archive_file}"
