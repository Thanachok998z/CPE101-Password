Password Game C v5 - Fullscreen (maximized)

1. Run build_windows.bat on Windows with MinGW GCC.
2. Window starts maximized. Password input is multiline, wraps long entries and scrolls vertically.
3. On final challenge, original password stays visible as non-selectable owner-painted text. Retype in a new field below; paste disabled there.
4. Rules with failed checks remain at the top. Mouse wheel scrolls the rule list.
5. Sign-in IDs are saved in data/students.txt.

Note: GUI needs real-Windows visual verification; logic can be tested with gcc -DTEST_MODE main.c.
