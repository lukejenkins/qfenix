#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

void SetMacOSDockIcon()
{
	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	wxFileName iconPath(exePath);
	/* Go from MacOS/ up to Contents/, then into Resources/ */
	iconPath.RemoveLastDir();
	iconPath.AppendDir("Resources");
	iconPath.SetFullName("qfenix.icns");

	if (!iconPath.FileExists())
		return;

	NSString *path = [NSString stringWithUTF8String:iconPath.GetFullPath().utf8_str().data()];
	NSImage *icon = [[NSImage alloc] initWithContentsOfFile:path];
	if (icon)
		[NSApp setApplicationIconImage:icon];
}
#endif
