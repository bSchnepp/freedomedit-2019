#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <gtk/gtk.h>

#include "syntax.h"

typedef struct EditorPane
{
	GtkWidget *scrolledContainer;
	GtkWidget *textView;
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

/* Placeholder, for now. */
void AssignRules(GtkTextBuffer *textbuffer)
{
	GtkTextTag *tag = gtk_text_buffer_create_tag(textbuffer, "genericTAG", 
		"foreground", "#FFFF00", 
		"family", "Inconsolata Regular", 
		"size-points", 15.0, 
	NULL);
	
	GtkTextTag *tag2 = gtk_text_buffer_create_tag(textbuffer, "genericTAG2", 
		"foreground", "#008844", 
		"background", "#000000",
		"family", "Inconsolata Regular", 
	NULL);
}


static void PositionChanged(GObject *theobject, GParamSpec *spec, gpointer edContext)
{
	GtkTextBuffer *textbuffer = GTK_TEXT_BUFFER(theobject);
	GtkTextMark *mark = gtk_text_buffer_get_insert(textbuffer);
	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(textbuffer, &iter, mark);
	GdkRectangle rect;
	/* Fix off by one issues. */
	gint xPos = gtk_text_iter_get_line(&iter) + 1;
	gint cPos = gtk_text_iter_get_line_offset(&iter) + 1;
	
	/* Initialize this *first* before our loop. This iter is at the first entry in the line. */
	gint lineOffset = gtk_text_iter_get_offset(&iter) - 1;
	
	GtkTextIter currentSpot = iter;
	GtkTextIter lineStart = iter;
	GtkTextIter wordStart = iter;
	
	
	
	/* To get the *exact* column, we need to go do a little funny loop. */
	/* As a hack, we synonymize tabs with 4 spaces of length. Not always true.*/
	uint32_t yPos = 1;
	uint32_t wordOffset = 1;
	bool wordStartReached = FALSE;
	while (gtk_text_iter_backward_char(&iter))
	{
		if (!wordStartReached && (gtk_text_iter_starts_line(&iter) || gtk_text_iter_starts_word(&iter)))
		{
			wordStart = iter;
			wordStartReached = TRUE;
		} else if (!wordStartReached) {
			wordOffset++;
		}
		gunichar ch = gtk_text_iter_get_char(&iter);
		
		if (ch == '\n')
		{
			break;
		} else if (ch == '\t') {
			yPos += 4;
		} else {
			yPos++;
		}
	}
	lineOffset -= yPos - 1;
	/* TODO  FIXME: This stuff here is TEMPORARY, to see how we do syntax highlighting without GtkSourceView.
		We should refactor this to create rules _before_ opening, instead of
		trying over and over and over again.
	 */
	gtk_text_buffer_get_iter_at_offset(textbuffer, &lineStart, lineOffset);
	gchar *currentWord = gtk_text_buffer_get_text(textbuffer, &wordStart, &currentSpot, true);
	
	gtk_text_buffer_apply_tag_by_name(textbuffer, "genericTAG", &wordStart, &currentSpot);
	if (strcmp(currentWord, "for") == 0)
	{
		gtk_text_buffer_apply_tag_by_name(textbuffer, "genericTAG2", &wordStart, &currentSpot);
	}
	
	EditorContext *context = (EditorContext*)edContext;
	/* 
		Don't allow more than 64 chars. This should be more than enough for everything.
		If you *somehow* manage to open a file with enough newlines or columns
		to cause this to overflow, then it just causes the counter to go
		beszerk (last few numbers sort of ignored.) This has been tested
		by changing to buffer size 10 and copy size 9.
	 */
	char bufX[65] = {'\0'};
	char bufY[65] = {'\0'};
	char bufC[65] = {'\0'};
	
	snprintf(bufX, 64, "Line %u", xPos);
	snprintf(bufY, 64, "Column %u", yPos);
	snprintf(bufC, 64, "Character %u", cPos);
	
	gtk_label_set_text(GTK_LABEL(context->status.rowPos), bufX);
	gtk_label_set_text(GTK_LABEL(context->status.colPos), bufY);
	gtk_label_set_text(GTK_LABEL(context->status.charPos), bufC);
}


static void AddPane(EditorContext *context)
{
	uint32_t panesCount = context->panesCount + 1;
	if (panesCount <= 1)
	{
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(context->tabbedPane), FALSE);
	} else {
		gtk_notebook_set_show_tabs(GTK_NOTEBOOK(context->tabbedPane), TRUE);
	}
	EditorPane *panes = context->panes;
	if (panesCount != 0)
	{
		panes = realloc(context->panes, sizeof(EditorPane) * panesCount);
	} else {
		panes = malloc(sizeof(EditorPane));
	}
	EditorPane pane;
	pane.textView = gtk_text_view_new();
	pane.FileLocation = NULL;
	pane.FileName = "";
	pane.scrolledContainer = gtk_scrolled_window_new(NULL, NULL);
	AssignRules(gtk_text_view_get_buffer(GTK_TEXT_VIEW(pane.textView)));
	g_signal_connect(gtk_text_view_get_buffer(GTK_TEXT_VIEW(pane.textView)), "notify::cursor-position", G_CALLBACK(PositionChanged), context);
	panes[panesCount - 1] = pane;
	
	gtk_container_add(GTK_CONTAINER(pane.scrolledContainer), panes[panesCount - 1].textView);
	gtk_notebook_append_page(GTK_NOTEBOOK(context->tabbedPane), pane.scrolledContainer, gtk_label_new(pane.FileName));

	context->panesCount = panesCount;
	context->panes = panes;
}

static void CreateMenuBar(EditorContext *context)
{
	context->menuBar.menuBarActual = gtk_menu_bar_new();
	
	context->menuBar.fileMenu = gtk_menu_item_new_with_label("File");
	context->menuBar.editMenu = gtk_menu_item_new_with_label("Edit");
	context->menuBar.viewMenu = gtk_menu_item_new_with_label("View");
	
	GtkWidget *fileMenuActual = gtk_menu_new();
	GtkWidget *editMenuActual = gtk_menu_new();
	GtkWidget *viewMenuActual = gtk_menu_new();
	
	/* File menu... */
	GtkWidget *fileMenuNew = gtk_menu_item_new_with_mnemonic("_New");
	GtkWidget *fileMenuOpen = gtk_menu_item_new_with_mnemonic("_Open");
	GtkWidget *fileMenuSave = gtk_menu_item_new_with_mnemonic("_Save");
	GtkWidget *fileMenuAddToSourceControl = gtk_menu_item_new_with_label("Version Control");
	GtkWidget *fileMenuProject = gtk_menu_item_new_with_label("Project");
	GtkWidget *fileMenuExit = gtk_menu_item_new_with_label("Exit");
	
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuNew);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuOpen);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuSave);
	
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), gtk_separator_menu_item_new());
	
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuAddToSourceControl);
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuProject);
	
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), gtk_separator_menu_item_new());
	
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenuActual), fileMenuExit);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(context->menuBar.fileMenu), fileMenuActual);
	gtk_container_add(GTK_CONTAINER(context->menuBar.menuBarActual), context->menuBar.fileMenu);
	gtk_container_add(GTK_CONTAINER(context->menuBar.menuBarActual), context->menuBar.editMenu);
	gtk_container_add(GTK_CONTAINER(context->menuBar.menuBarActual), context->menuBar.viewMenu);
}

static void CreateToolsBar(EditorContext *context)
{
	GtkWidget *toolbar = gtk_toolbar_new();
	
	/* TODO: Replace with a nice, green arrow. This is a placeholder, as usual. */
	GtkToolItem *runButton = gtk_tool_button_new(gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_LARGE_TOOLBAR), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), runButton, -1);

	GtkWidget *targetArchSelector = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(targetArchSelector), NULL, "x86_64");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(targetArchSelector), NULL, "i686");
	gtk_combo_box_set_active(GTK_COMBO_BOX(targetArchSelector), 0);	
	
	GtkWidget *buildTypeSelector = gtk_combo_box_text_new();
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(buildTypeSelector), NULL, "Debug");
	gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(buildTypeSelector), NULL, "Release");
	gtk_combo_box_set_active(GTK_COMBO_BOX(buildTypeSelector), 0);
	
	GtkWidget *rightSideContainer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(rightSideContainer), targetArchSelector);
	gtk_container_add(GTK_CONTAINER(rightSideContainer), buildTypeSelector);
	gtk_container_add(GTK_CONTAINER(rightSideContainer), toolbar);
	
	gtk_header_bar_pack_end(GTK_HEADER_BAR(context->titleBar), rightSideContainer);
}

void CreateStatusBar(EditorContext *context)
{
	/* Status bar here. Again, placeholder to get a good UI down. */
	GtkWidget *statusbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *progBar = gtk_progress_bar_new();
	/* Perpetually 75% done, for now. */
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progBar), 0.75);
	
	gtk_box_pack_start(GTK_BOX(statusbar), progBar, TRUE, TRUE, 0);
	
	/* Add a spacer. */
	gtk_container_add(GTK_CONTAINER(statusbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	
	GtkWidget *lineStatus = gtk_label_new("Line 1");
	GtkWidget *colStatus = gtk_label_new("Column 1");
	GtkWidget *charStatus = gtk_label_new("Character 1");
	GtkWidget *inputStatus = gtk_label_new("Insert Mode");
	
	gtk_box_pack_start(GTK_BOX(statusbar), lineStatus, FALSE, FALSE, 12);
	gtk_container_add(GTK_CONTAINER(statusbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_pack_start(GTK_BOX(statusbar), colStatus, FALSE, FALSE, 12);
	gtk_container_add(GTK_CONTAINER(statusbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_pack_start(GTK_BOX(statusbar), charStatus, FALSE, FALSE, 12);
	gtk_container_add(GTK_CONTAINER(statusbar), gtk_separator_new(GTK_ORIENTATION_VERTICAL));
	gtk_box_pack_start(GTK_BOX(statusbar), inputStatus, FALSE, FALSE, 12);
	
	context->status.progBar = progBar;
	context->status.rowPos = lineStatus;
	context->status.colPos = colStatus;
	context->status.charPos = charStatus;
	context->status.inputMode = inputStatus;

	gtk_box_pack_end(GTK_BOX(context->currentPane), statusbar, FALSE, FALSE, 0);
}

static void ActivatePrimary(GtkApplication *app, gpointer edContext)
{
	/* reinterpret_cast our data we want here. */
	EditorContext* context = (EditorContext*)edContext;
	
	/* Initially, the window needs to be created. Go do that. */
	context->window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(context->window), "FreedomEdit");
	gtk_window_set_default_size(GTK_WINDOW(context->window), 800, 600);
	
	/* Update the header bar and replace the default one. */
	context->titleBar = gtk_header_bar_new();
	gtk_header_bar_set_title(GTK_HEADER_BAR(context->titleBar), "FreedomEdit");
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(context->titleBar), "Untitled Project");
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(context->titleBar), TRUE);
	gtk_window_set_titlebar(GTK_WINDOW(context->window), context->titleBar);
	
	/* Set up the vertical box where we put widgets.*/
	context->currentPane = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* First thing we add is a notebook. */
	context->tabbedPane = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(context->currentPane), context->tabbedPane, TRUE, TRUE, 0);
	
	CreateMenuBar(context);
	AddPane(context);
	gtk_header_bar_pack_start(GTK_HEADER_BAR(context->titleBar), context->menuBar.menuBarActual);
	
	CreateToolsBar(context);
	CreateStatusBar(context);



	gtk_container_add(GTK_CONTAINER(context->window), context->currentPane);
	gtk_widget_show_all(context->window);
}


int main(int argc, char** argv)
{
	GtkApplication *app = gtk_application_new("com.gmail.bschneppdev.freedomedit", G_APPLICATION_FLAGS_NONE);
	EditorContext context;
	context.panesCount = 0;
	context.panes = NULL;
	g_signal_connect(app, "activate", G_CALLBACK(ActivatePrimary), &context);

	int appStatus = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return appStatus;
}
