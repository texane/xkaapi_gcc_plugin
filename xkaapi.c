/* todo:
   . how to bind a pragma to a scope
   -> needed for alloca(name)
   . associate pragma with a locus
   -> needed to retrieve the location
   . describe grammar
 */

#include "config.h"
#include "plugin.h"
#include "plugin-version.h"
#include "gcc-plugin.h"
#include "coretypes.h"
#include "toplev.h"
#include "gimple.h"
#include "tree.h"
#include "tree-pass.h"
#include "cpplib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


/* mandatory */
int plugin_is_GPL_compatible;


/* debugging */
#if 0
#define TRACE() printf("%s::%u\n", __FUNCTION__, __LINE__)
#else
#define TRACE()
#endif


/* pragma expression tracker
 */

typedef struct pragma_expr
{
  /* locus */
  const char* file;
  unsigned int line;

  /* datastruct */
  struct pragma_expr* next;

} pragma_expr_t;

static pragma_expr_t* pragma_exprs;

static pragma_expr_t* alloc_pragma_expr(void)
{
  /* use xmalloc: http://gcc.gnu.org/ml/gcc/2004-09/msg00966.html */
  pragma_expr_t* const x = xmalloc(sizeof(pragma_expr_t));
  if (x != NULL) x->next = NULL;
  return x;
}

static void add_pragma_expr(pragma_expr_t* x)
{
  x->next = pragma_exprs;
  pragma_exprs = x;
}

static const pragma_expr_t* find_pragma_expr
(const char* file, unsigned int line)
{
  const pragma_expr_t* x = pragma_exprs;

  for (; x; x = x->next)
    if (x->line == line && !strcmp(x->file, file))
      return x;

  return NULL;
}

static void free_pragma_exprs(pragma_expr_t* x)
{
  while (x)
  {
    pragma_expr_t* const tmp = x;
    x = x->next;
    free(tmp);
  }
}


/* pragma handlers
   http://gcc.gnu.org/onlinedocs/cppinternals
   gcc/gcc/testsuite/g++.dg/plugin/pragma_plugin.c
 */

/* todo
   parse_pragma_clause_list
   parse_pragma_clause
 */

#if 0 /* unused, tokens not consumed */

static int get_pragma_line
(struct cpp_reader* reader, unsigned char* bufpos, size_t buflen)
{
  /* return a 0 terminated string */

  const struct cpp_token* tok;
  unsigned char* newpos;
  size_t toklen;

  while (1)
  {
    tok = cpp_get_token(reader);
    if (tok->type == CPP_PRAGMA_EOL || tok->type == CPP_EOF)
    {
      *bufpos = 0;
      return 0;
    }

    toklen = cpp_token_len(tok);
    if (toklen >= buflen) return -1;

    newpos = cpp_spell_token(reader, tok, bufpos, 0);
    buflen -= (newpos - bufpos);
    bufpos = newpos;
  }

  return -1;
}

#endif /* unused, tokens not consumed */

static int parse_pragma_line
(struct cpp_reader* reader)
{
  /* return a 0 terminated string */

  tree expr;

  while (1)
  {
    /* in cpplib.h */
    const enum cpp_ttype type = pragma_lex(&expr);

    /* locus */
    {
      const char* const file = LOCATION_FILE(input_location);
      const unsigned int line = LOCATION_LINE(input_location);
      printf("PRAGMA: .%s/%u.\n", file, line);
    }

    switch (type)
    {
    case CPP_PRAGMA_EOL:
    case CPP_EOF:
      goto on_done;
      break ;

    case CPP_COMMA:
      printf(",");
      break ;

    case CPP_OPEN_PAREN:
      printf("(");
      break ;

    case CPP_CLOSE_PAREN:
      printf(")");
      break ;

    case CPP_STRING:
      printf("str<%s>", TREE_STRING_POINTER(expr));
      break ;

    case CPP_NAME:
      printf("nam<%s>", IDENTIFIER_POINTER(expr));
      break ;

    default:
      printf("unk<%u>", type);
      break ;
    }
  }

 on_done:
  printf("\n");
  return 0;
}

static void handle_common_pragma(struct cpp_reader* reader)
{
  if (parse_pragma_line(reader) == -1)
  {
    printf("parse_pragma_line() == -1\n");
    return ;
  }
}

static void handle_task_pragma(struct cpp_reader* reader)
{
  TRACE();
  handle_common_pragma(reader);
}

static void handle_data_pragma(struct cpp_reader* reader)
{
  TRACE();
  handle_common_pragma(reader);
}


/* pragma registration */

static int register_pragmas(void)
{
  TRACE();

#if defined(__cplusplus)
# define register_pragma cpp_register_pragma
#else
# define register_pragma c_register_pragma
#endif

#define XKAAPI_SPACE_STRING "xkaapi"
  /* dont know why, the space string must be global */
  register_pragma(XKAAPI_SPACE_STRING, "task", handle_task_pragma);
  register_pragma(XKAAPI_SPACE_STRING, "data", handle_data_pragma);

  return 0;
}


/* gimple pass
 */

static unsigned int on_execute_pass(void)
{
  basic_block bb;
  gimple_stmt_iterator gsi;

  TRACE();

  if (DECL_NAME(cfun->decl))
  {
    printf("decl %s\n", IDENTIFIER_POINTER(DECL_NAME(cfun->decl)));
  }

  /* locus */
  {
    const char* file = EXPR_FILENAME(cfun->decl);
    const unsigned int line = EXPR_LINENO(cfun->decl);
    printf("DECL locus: .%s/%u.\n", file, line);
  }

  FOR_EACH_BB(bb)
  {
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
    {
      const_gimple stmt = gsi_stmt(gsi);
      const enum gimple_code code = gimple_code(stmt);
      const char* type = "STMT";

      if (code == GIMPLE_CALL)
	type = "CALL";

      if (gimple_has_location(stmt))
      {
	const location_t loc = gimple_location(stmt);
	printf
	(
	 "%s locus: .%s/%u.\n",
	 type, LOCATION_FILE(loc), LOCATION_LINE(loc)
	);
      }
    }
  }

  return 0;
}

static void register_mein_pass(void)
{
  /* from tree-pass.h */

  struct opt_pass opt_pass;
  struct register_pass_info pass_info;

  opt_pass.type = GIMPLE_PASS;
  opt_pass.name = "xkaapi_pass";
  opt_pass.gate = NULL; /* execute always */
  opt_pass.execute = on_execute_pass;
  opt_pass.sub = NULL;
  opt_pass.next = NULL;
  opt_pass.tv_id = TV_NONE;
  opt_pass.properties_required = PROP_gimple_any;
  opt_pass.properties_provided = 0;
  opt_pass.properties_destroyed = 0;
  opt_pass.todo_flags_start = 0;
  opt_pass.todo_flags_finish = 0;

  /* before the first lowering pass, scope conserved... */
  pass_info.pass = &opt_pass;
  pass_info.reference_pass_name = "ssa";
  pass_info.ref_pass_instance_number = 1;
  pass_info.pos_op = PASS_POS_INSERT_BEFORE;

  register_callback("xkaapi", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
  /* or: register_pass(&pass_info); */
}


/* plugin callbacks
 */

static void on_finish
(void* gcc_data, void* user_data)
{
  TRACE();
  free_pragma_exprs(pragma_exprs);
}

static void register_callbacks(void)
{
#define PLUGIN_NAME "xkaapi"
  register_callback(PLUGIN_NAME, PLUGIN_FINISH, on_finish, NULL);
}


/* plugin entrypoint
 */

int plugin_init
(
 struct plugin_name_args* plugin_info,
 struct plugin_gcc_version* version
)
{
  TRACE();

  /* todo: disablable thru argument passing */
  if (!plugin_default_version_check(version, &gcc_version))
  {
    printf("compiler version does not match\n");
    return 1;
  }

  register_callbacks();
  register_pragmas();
  register_mein_pass();

  return 0;
}
