/* -*- mode: c; style: linux -*- */

/* symbol.c
 * Copyright (C) 2000 Helix Code, Inc.
 *
 * Written by Bradford Hovinen <hovinen@helixcode.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "symbol.h"
#include "glyph-layout.h"

enum {
	ARG_0,
	ARG_GLYPH
};

struct _SymbolPrivate 
{
	guint glyph;
};

static MathUnitClass *parent_class;

static GlyphLayout *layout;

static void symbol_init        (Symbol *symbol);
static void symbol_class_init  (SymbolClass *class);

static void symbol_set_arg     (GtkObject *object, 
				GtkArg *arg, 
				guint arg_id);
static void symbol_get_arg     (GtkObject *object, 
				GtkArg *arg, 
				guint arg_id);

static void symbol_finalize    (GtkObject *object);

static const Layout *symbol_get_layout (MathObject *math_object);

guint
symbol_get_type (void)
{
	static guint symbol_type = 0;

	if (!symbol_type) {
		GtkTypeInfo symbol_info = {
			"Symbol",
			sizeof (Symbol),
			sizeof (SymbolClass),
			(GtkClassInitFunc) symbol_class_init,
			(GtkObjectInitFunc) symbol_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		symbol_type = 
			gtk_type_unique (math_unit_get_type (), 
					 &symbol_info);
	}

	return symbol_type;
}

static void
symbol_init (Symbol *symbol)
{
	symbol->p = g_new0 (SymbolPrivate, 1);
}

static void
symbol_class_init (SymbolClass *class) 
{
	GtkObjectClass *object_class;
	MathObjectClass *math_object_class;

	gtk_object_add_arg_type ("Symbol::glyph",
				 GTK_TYPE_INT,
				 GTK_ARG_READWRITE,
				 ARG_GLYPH);

	object_class = GTK_OBJECT_CLASS (class);
	object_class->finalize = symbol_finalize;
	object_class->set_arg = symbol_set_arg;
	object_class->get_arg = symbol_get_arg;

	math_object_class = MATH_OBJECT_CLASS (class);
	math_object_class->get_layout = symbol_get_layout;

	parent_class = MATH_UNIT_CLASS
		(gtk_type_class (math_unit_get_type ()));

	layout = GLYPH_LAYOUT (glyph_layout_new ());
}

static void
symbol_set_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	Symbol *symbol;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_SYMBOL (object));

	symbol = SYMBOL (object);

	switch (arg_id) {
	case ARG_GLYPH:
		symbol->p->glyph = GTK_VALUE_INT (*arg);
		break;

	default:
		g_warning ("Bad argument set");
		break;
	}
}

static void
symbol_get_arg (GtkObject *object, GtkArg *arg, guint arg_id) 
{
	Symbol *symbol;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_SYMBOL (object));

	symbol = SYMBOL (object);

	switch (arg_id) {
	case ARG_GLYPH:
		GTK_VALUE_INT (*arg) = symbol->p->glyph;
		break;

	default:
		g_warning ("Bad argument get");
		break;
	}
}

static void
symbol_finalize (GtkObject *object) 
{
	Symbol *symbol;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_SYMBOL (object));

	symbol = SYMBOL (object);

	g_free (symbol->p);
}

GtkObject *
symbol_new (guint glyph) 
{
	return gtk_object_new (symbol_get_type (),
			       "glyph", glyph,
			       NULL);
}

/**
 * symbol_set_glyph:
 * @symbol: 
 * @glyph: An ISO10346-encoded character
 * 
 * Set the glyph for this symbol to a new value
 **/

void
symbol_set_glyph (Symbol *symbol, guint glyph)
{
	g_return_if_fail (symbol != NULL);
	g_return_if_fail (IS_SYMBOL (symbol));

	symbol->p->glyph = glyph;
}

/**
 * symbol_get_glyph:
 * @symbol: 
 * 
 * Get the glyph for this symbol
 * 
 * Return value: The glyph, ISO10346-encoded
 **/

guint
symbol_get_glyph (Symbol *symbol)
{
	g_return_val_if_fail (symbol != NULL, 0);
	g_return_val_if_fail (IS_SYMBOL (symbol), 0);

	return symbol->p->glyph;
}

static const Layout *
symbol_get_layout (MathObject *math_object)
{
	return LAYOUT (glyph_layout_new ());
}
