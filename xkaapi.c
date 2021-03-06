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
#include "tree-flow.h"
#include "cpplib.h"
#include "c-common.h"

#include <stdint.h>
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


/* decls initialization
 */

static tree kaapi_pushdata_aligned_decl = NULL;
static tree kaapi_pushtask_decl = NULL;
static tree kaapi_barrier_decl = NULL;

static void init_decls(void)
{
  static unsigned int is_once = 0;
  tree type;

  if (is_once) return ;
  is_once = 1;

  type = build_function_type_list
    (ptr_type_node, size_type_node, NULL_TREE);
  kaapi_pushdata_aligned_decl =
    build_fn_decl("kaapi_pushdata_aligned", type);

  type = build_function_type_list
    (void_type_node, ptr_type_node, ptr_type_node, NULL_TREE);
  kaapi_pushtask_decl =
    build_fn_decl("kaapi_pushtask", type);

  type = build_function_type_list
    (void_type_node, void_type_node, NULL_TREE);
  kaapi_barrier_decl =
    build_fn_decl("kaapi_barrier", type);
}


/* pragma expression tracker
 */

typedef enum pragma_type
{
  PRAGMA_TYPE_TASK = 0,
  PRAGMA_TYPE_DATA
} pragma_type_t;

typedef struct pragma_expr
{
  /* locus */
  char* file;
  unsigned int line;

  pragma_type_t type;

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

static inline pragma_expr_t* alloc_add_pragma_expr(void)
{
  pragma_expr_t* const x = alloc_pragma_expr();
  if (x != NULL) add_pragma_expr(x);
  return x;
}

static const pragma_expr_t* find_pragma_expr
(const char* file, unsigned int line)
{
  const pragma_expr_t* x = pragma_exprs;

  for (; x; x = x->next)
    if ((x->line == line) && !strcmp(x->file, file))
      return x;

  return NULL;
}

static inline const pragma_expr_t* find_data_pragma_expr
(const char* file, unsigned int line)
{
  const pragma_expr_t* x = find_pragma_expr(file, line);
  if (x != NULL && x->type != PRAGMA_TYPE_DATA) x = NULL;
  return x;
}

static inline const pragma_expr_t* find_task_pragma_expr
(const char* file, unsigned int line)
{
  const pragma_expr_t* x = find_pragma_expr(file, line);
  if (x != NULL && x->type != PRAGMA_TYPE_TASK) x = NULL;
  return x;
}

static void free_pragma_exprs(void)
{
  pragma_expr_t* x = pragma_exprs;

  while (x)
  {
    pragma_expr_t* const tmp = x;
    x = x->next;
    free(tmp->file);
    free(tmp);
  }

  pragma_exprs = NULL;
}


/* function tracker
   todo: hash table, gcc splay-trees.h
 */

typedef struct tracked_func
{
  /* assembler name */
  char* name;

  /* related pragma expression */
  const pragma_expr_t* pragma_expr;

  /* datastruct */
  struct tracked_func* next;

} tracked_func_t;

static tracked_func_t* tracked_funcs;

static inline tracked_func_t* add_tracked_func(void)
{
  tracked_func_t* const tf = xmalloc(sizeof(tracked_func_t));
  tf->next = tracked_funcs;
  tracked_funcs = tf;
  return tf;
}

static const tracked_func_t* find_tracked_func(const char* name)
{
  /* find by name */

  const tracked_func_t* pos;
  for (pos = tracked_funcs; pos; pos = pos->next)
    if (!strcmp(pos->name, name)) break ;
  return pos;
}

static void free_tracked_funcs(void)
{
  tracked_func_t* pos = tracked_funcs;

  while (pos)
  {
    tracked_func_t* const tmp = pos;
    pos = pos->next;
    free(tmp->name);
    free(tmp);
  }

  tracked_funcs = NULL;
}


/* pragma handlers
   http://gcc.gnu.org/onlinedocs/cppinternals
   gcc/gcc/testsuite/g++.dg/plugin/pragma_plugin.c
 */

/* todo
   parse_pragma_clause_list
   parse_pragma_clause
 */

static int parse_pragma_line
(struct cpp_reader* reader, pragma_type_t type)
{
  pragma_expr_t* const px = alloc_add_pragma_expr();

  if (px == NULL) return 0;
  px->type = type;
  px->file = xstrdup(LOCATION_FILE(input_location));
  px->line = LOCATION_LINE(input_location);

#if 0 /* debug */
  printf("pragma: %s/%u\n", px->file, px->line);
#endif

#if 0 /* TODO */

  while (1)
  {
    /* in cpplib.h */
    tree expr;
    const enum cpp_ttype ttype = pragma_lex(&expr);

    switch (ttype)
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
      printf("unk<%u>", ttype);
      break ;
    }
  }

 on_done:
  printf("\n");

#endif /* TODO */


  return 0;
}

static void handle_common_pragma
(struct cpp_reader* reader, pragma_type_t type)
{
  if (parse_pragma_line(reader, type) == -1)
  {
    printf("parse_pragma_line() == -1\n");
    return ;
  }
}

static void handle_task_pragma(struct cpp_reader* reader)
{
  TRACE();
  handle_common_pragma(reader, PRAGMA_TYPE_TASK);
}

static void handle_data_pragma(struct cpp_reader* reader)
{
  TRACE();
  handle_common_pragma(reader, PRAGMA_TYPE_DATA);
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
   see gimple-pretty-print.c on how to iterate
 */

static const char* get_called_name(const_gimple stmt)
{
  /* return the low level, assembler name if possible */

  tree op = gimple_call_fn(stmt);

  if (TREE_CODE(op) == NON_LVALUE_EXPR)
    op = TREE_OPERAND(op, 0);

 do_again:
  if (TREE_CODE(op) == ADDR_EXPR)
  {
    op = TREE_OPERAND(op, 0);
    goto do_again;
  }
  else if (TREE_CODE(op) == FUNCTION_DECL)
  {
    if (DECL_ASSEMBLER_NAME(op))
      return IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(op));
    else if (DECL_NAME(op))
      return IDENTIFIER_POINTER(DECL_NAME(op));
  }

  return "_anonymous_";
}

#if 0 /* todo */
static void handle_task_decl(void)
{
}
#endif /* todo */

static void track_pragmed_func
(const char* file, unsigned int line, const char* name)
{
  /* look for a corresponding task pragma */

  const pragma_expr_t* expr;
  tracked_func_t* tf;

  if (line == 0) return ;

  expr = find_task_pragma_expr(file, line - 1);
  if (expr == NULL) return ;

#if 0 /* debug */
  printf("[x] pragmed function\n");
#endif

  tf = add_tracked_func();
  tf->pragma_expr = expr;
  tf->name = xstrdup(name);

  /* todo: handle_task_decl(); */
}

static tree create_dummy_adapter(void)
{
  /* create function type */
  static tree type = NULL;
  static tree decl = NULL;

  if (decl != NULL) return decl;

  /* create the type void fu(void)*/
  type = build_function_type_list
    (void_type_node, ptr_type_node, ptr_type_node, NULL_TREE);

  /* create the function decl */
  decl = build_fn_decl("__xkaapi_dummy_adapter", type);

  return decl;
}

extern tree add_new_static_var(tree type);

static void gen_alloca_stmts(gimple_seq* stmts_, tree* sp_)
{
  /* doc: tree-complex.c, c-common.c, omp-low.c */

  /* create the stmt sequence so that:
     *sp = alloca(sizeof(type));
     type: the type to be stacked
     stmts: the allocation sequence
     sp: the allocated area ptr
   */

  gimple call;
  gimple_seq stmts;
  tree binop;
  tree mul;
  tree val;
  tree count;
  tree sizeuf;
  tree sp;
  tree ref;
  gimple assign;
  gimple_stmt_iterator gsi;

  /* build an empty sequence */
  stmts = gimple_seq_alloc();
  gsi = gsi_last(stmts);

#if 0 /* fixme */
  sp = create_tmp_var(ptr_type_node, NULL);
#else
  /* from varpool.c */
  sp = add_new_static_var(ptr_type_node);
#endif

  /* sp = __builtin_alloca(count * sizeof(uintptr_t)); */
  count = build_int_cst(integer_type_node, 2);
  sizeuf = build_int_cst(integer_type_node, sizeof(uintptr_t));
  mul = build2(MULT_EXPR, integer_type_node, count, sizeuf);
  /* call = gimple_build_call(built_in_decls[BUILT_IN_ALLOCA], 1, mul); */
  call = gimple_build_call(kaapi_pushdata_aligned_decl, 1, mul);
  gimple_call_set_lhs(call, sp);
  gsi_insert_after(&gsi, call, GSI_CONTINUE_LINKING);

  /* *(sp + 0) = 42; */
  ref = build1(INDIRECT_REF, TREE_TYPE(sp), sp);
  val = build_int_cst(ptr_type_node, 0xdeadc0d3);
  assign = gimple_build_assign(ref, val);
  gsi_insert_after(&gsi, assign, GSI_CONTINUE_LINKING);

  /* *(sp + 8) = 24 */
  binop = build_binary_op
    (0, PLUS_EXPR, convert(integer_type_node, sp), sizeuf, 0);
  ref = build1(INDIRECT_REF, TREE_TYPE(binop), binop);
  val = build_int_cst(ptr_type_node, 0xdeadc003);
  assign = gimple_build_assign(ref, val);
  gsi_insert_after(&gsi, assign, GSI_CONTINUE_LINKING);

  /* affect results */
  *sp_ = sp;
  *stmts_ = stmts;
}

static void handle_task_call
(gimple_stmt_iterator gsi, const tracked_func_t* tf)
{
  /* iterator points to the call instruction */

  /* . generate the following statement list:
     generate type T;
     declare T* var;
     var = call __xkaapi_pushdata(sizeof(T));
     foreach args, var->member = local_value;
     call __xkaapi_pushtask();
     call __xkaapi_barrier();
   */

  tree dummy_adapter;
  tree sp;
  tree thread;
  gimple_seq stmts;
  gimple call;

  init_decls();

  /* kaapi_pushdata_aligned */
  gen_alloca_stmts(&stmts, &sp);
  gsi_insert_seq_before(&gsi, stmts, GSI_SAME_STMT);
  /* fixme: gimple_seq_free(alloca_stmts); */

  /* generate the adapter routine */
  /* dummy_adapter = create_dummy_adapter(); */

  /* kaapi_pushtask(self_thread, sp); */
  thread = build_int_cst(ptr_type_node, 0xdeadc0c0);
  call = gimple_build_call(kaapi_pushtask_decl, 2, sp, thread);
  gsi_insert_before(&gsi, call, GSI_SAME_STMT);

  /* kaapi_barrier(); */
  call = gimple_build_call(kaapi_barrier_decl, 0);
  gsi_replace(&gsi, call, true);
}

static unsigned int on_execute_pass(void)
{
  basic_block bb;
  gimple_stmt_iterator gsi;
  const char* name;
  const char* file = EXPR_FILENAME(cfun->decl);
  const unsigned int line = EXPR_LINENO(cfun->decl);

  TRACE();

  if (DECL_ASSEMBLER_NAME(cfun->decl) == NULL)
  {
    printf("--- skipping anonymous function\n");
    return 0;
  }

  name = IDENTIFIER_POINTER(DECL_ASSEMBLER_NAME(cfun->decl));

#if 0 /* debug */
  printf("--- passing on function: %s\n", name);
#endif

  track_pragmed_func(file, line, name);

  FOR_EACH_BB(bb)
  {
    for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
    {
      const_gimple stmt = gsi_stmt(gsi);
      const enum gimple_code code = gimple_code(stmt);

      if (code == GIMPLE_CALL)
      {
	const char* const name = get_called_name(stmt);
	const tracked_func_t* const tf = find_tracked_func(name);

	printf("CALL%s: %s()\n", tf ? "_TASK" : "", name);

	if (tf != NULL) handle_task_call(gsi, tf);
      }

#if 0 /* debug */
      if (gimple_has_location(stmt))
      {
	const location_t loc = gimple_location(stmt);

	const char* type = "STMT";
	if (code == GIMPLE_CALL)

	printf
	(
	 "%s locus: .%s/%u.\n",
	 type, LOCATION_FILE(loc), LOCATION_LINE(loc)
	);
      }
#endif /* debug */
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
  opt_pass.todo_flags_finish = TODO_dump_func;

  /* before the first lowering pass, scope conserved... */
  pass_info.pass = &opt_pass;
  pass_info.reference_pass_name = "early_optimizations";
  /* pass_info.reference_pass_name = "ssa"; */
  pass_info.ref_pass_instance_number = 1;
  pass_info.pos_op = PASS_POS_INSERT_BEFORE;

  register_callback("xkaapi", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
  /* or: register_pass(&pass_info); */
}


/* datastruct initialization
 */

static void init_datastructs(void)
{
  pragma_exprs = NULL;
  tracked_funcs = NULL;
}

static void fini_datastructs(void)
{
  free_pragma_exprs();
  free_tracked_funcs();
}


/* plugin callbacks
 */

static void on_finish
(void* gcc_data, void* user_data)
{
  TRACE();
  fini_datastructs();
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

  init_datastructs();

  register_callbacks();
  register_pragmas();
  register_mein_pass();

  return 0;
}
