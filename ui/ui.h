#include <gtk/gtk.h>

typedef struct EditorPane
{
	GtkWidget *scrolledContainer;
	GtkWidget *textView;
	GtkWidget *sourceMap;
	const char* FileLocation;
	const char* FileName;
}EditorPane;

typedef struct MenuBar
{
	GtkWidget *menuBarActual;

	GtkWidget *fileMenu;
	GtkWidget *editMenu;
	GtkWidget *viewMenu;
}MenuBar;

typedef struct StatusBar
{
	GtkWidget *progBar;
	GtkWidget *rowPos;
	GtkWidget *colPos;
	GtkWidget *charPos;
	GtkWidget *inputMode;
	bool replaceMode;
}StatusBar;

typedef struct EditorContext
{
	GtkWidget *window;
	GtkWidget *titleBar;
	GtkWidget *currentPane;

	uint32_t panesCount;
	EditorPane *panes;

	/* "private" content. */
	GtkWidget *tabbedPane;	/* GtkNotebook */
	MenuBar menuBar;
	StatusBar status;
}EditorContext;

