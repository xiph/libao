/*
 * $Id: uiutil.c,v 1.1 2001/09/05 19:10:01 cwolf Exp $
 * $Name:  $
 *
 * Get the window handle of the controlling console of a console-mode
 * application.
 *
 * This came right off Microsoft's MSDN library, so I doubt there's
 * less kludgy way to handle this.
 */
#define MY_BUFSIZE 1024 // Buffer size for console window titles.

HWND getConsoleWindow()
{
	HWND hwndFound;         // This is what is returned to the caller.
  char pszNewWindowTitle[MY_BUFSIZE]; // Contains fabricated
                                      // WindowTitle.
  char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
                                      // WindowTitle.

       // Fetch current window title.
  GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

       // Format a "unique" NewWindowTitle.
  (void)wsprintf(pszNewWindowTitle,"%d/%d", GetTickCount(), 
                                            GetCurrentProcessId());

       // Change current window title.
  SetConsoleTitle(pszNewWindowTitle);

       // Ensure window title has been updated.
  Sleep(40);

       // Look for NewWindowTitle.
  hwndFound=FindWindow(NULL, pszNewWindowTitle);

       // Restore original window title.
  SetConsoleTitle(pszOldWindowTitle);

  return(hwndFound);
} 
