/******************************************************************************
 * spellhell.c 
 *
 * spellhell - GCC Spell Checking Plugin
 *
 * Copyright (C) 2011, 2014 Matt Davis (enferex) 
 * of 757Labs mattdavis9@gmail.com
 *
 * spellhell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * spellhell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with spellhell.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include <aspell.h>
#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <tree.h>
#include <tree-pass.h>
#include <cgraph.h>
#include <gimple-expr.h>
#include <tree-ssa-alias.h>
#include <internal-fn.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <rtl.h>
#include <expr.h>


/* Locale to use when spell checking */
#define LOCALE "en_US"


/* Predicate to check if a value is a-z or A-Z */
#define is_alpha(c) (((c)>64 && (c)<91) || ((c)>96 && (c)<123))


int plugin_is_GPL_compatible = 42;
static AspellSpeller *spellhell_g;


/* Help info about the plugin if one were to use gcc's --version --help */
static struct plugin_info spellhell_info =
{
    .version = "1.0",
    .help = "Spell check string literals"
};


/* We don't need to run any tests before we execute our plugin pass */
static bool spellhell_gate(void)
{
    return true;
}


static tree is_str_cst(tree node)
{
    tree offset;

    if (!node)
      return NULL_TREE;
    else if (TREE_CODE(node) == STRING_CST)
      return node;
    else 
      return string_constant(node, &offset);
}


static AspellSpeller *init_spellchecker(void)
{
    AspellConfig *cfg;
    AspellCanHaveError *err;

    /* Configure and instantiate a spell checker */
    cfg = new_aspell_config();
    aspell_config_replace(cfg, "lang", LOCALE);
    err = new_aspell_speller(cfg);
    if (aspell_error_number(err) != 0)
    {
        puts(aspell_error_message(err));
        return NULL;
    }

    return to_aspell_speller(err);
}


static void spell_check(const gimple *stmt, const_tree str)
{
    char buf[32] = {0};
    const char *data, *end;

    data = TREE_STRING_POINTER(str);
#ifdef DEBUG
    printf("Spell checking string: \'%s\'\n", data);
#endif

    while (*data)
    {
        /* Skip non alphas including whitespace */
        while (!is_alpha(data[0]))
        {
            if (data[0] == '\0')
              return;
            ++data;
        }

        /* Find the end of the word */
        end = data;
        while (is_alpha(end[0]))
          ++end;

        if ((end - data) > sizeof(buf))
          return;
       
        memcpy(buf, data, end - data);
        buf[end-data] = '\0';
        if (!(aspell_speller_check(spellhell_g, buf, end - data)))
          warning_at(gimple_location(stmt), 0, "%s (bad spelling)", buf);
        data = end;
    }
}


/* Callback once per function in the program being compiled */
static void spellhell_exec(void *gcc_data, void *user_data)
{
    unsigned i;
    struct function *fn;
    struct cgraph_node *node;
    tree str, op;
    basic_block bb;
    const gimple *stmt;
    gimple_stmt_iterator gsi;

    FOR_EACH_FUNCTION_WITH_GIMPLE_BODY(node)
    {
        if (!(fn = DECL_STRUCT_FUNCTION(node->decl)))
          continue;

        FOR_EACH_BB_FN(bb, fn)
        {
            for (gsi=gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
            {
                stmt = gsi_stmt(gsi);
                for (i=0; i<gimple_num_ops(stmt); ++i)
                  if ((op = gimple_op(stmt, i)) && (str = is_str_cst(op)))
                    spell_check(stmt, str);
            }
        }
    }
}


/* Return 0 on success or error code on failure */
int plugin_init(struct plugin_name_args   *info,  /* Argument info  */
                struct plugin_gcc_version *ver)   /* Version of GCC */
{
    /* Tell gcc we when to call us */
    register_callback("spellhell", PLUGIN_INFO, NULL, &spellhell_info);
    register_callback("spellhell", PLUGIN_ALL_IPA_PASSES_START,
                      spellhell_exec, NULL);

    /* Initialize our spell checker */
    if (!(spellhell_g = init_spellchecker()))
      return -1;

    return 0;
}
