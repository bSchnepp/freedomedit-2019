#include <stdint.h>
#include <gtk/gtk.h>

typedef uint8_t tristate_format_t;

#define TRISTATE_FORMAT_INVALID (0)
#define TRISTATE_FORMAT_IS_WORD (1)
#define TRISTATE_FORMAT_IS_AFTER (2)
#define TRISTATE_FORMAT_IS_WITHIN (3)

typedef struct WordFormat
{
	GtkTextTag *tag;
	const char *regexMatch;
}WordFormat;

typedef struct AfterFormat
{
	GtkTextTag *tag;
	const char *afterPattern;	/* ie, for '//' */
}AfterFormat;

typedef struct WithinFormat
{
	GtkTextTag *tag;
	const char* afterPattern;
	const char* beforePattern;
}WithinFormat;

typedef struct FormatRules
{
	tristate_format_t format;
	union
	{
		WordFormat asWord;
		AfterFormat asAfter;
		WithinFormat asWithin;
	};
	struct FormatRules *next;
}FormatRules;

/*
	"C"
	"CPP"
	"x86"
	"Rust"
	"GLSL"
	etc.
 */

typedef struct LanguageIndex
{
	/* HashMap would be ideal here. TODO. */
	const char* languageName;
	FormatRules rules;
	struct LanguageIndex *next;
}LanguageIndex;

void CreateKeywordIndex(const char *langIdentifier);
