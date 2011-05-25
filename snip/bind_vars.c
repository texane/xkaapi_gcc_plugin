static void print_bind_vars(tree var)
{
#ifndef DECL_CHAIN
# define DECL_CHAIN(__p) TREE_CHAIN(__p)
#endif
  for (; var; var = DECL_CHAIN(var))
  {
    printf("bind_var %s\n", IDENTIFIER_POINTER(DECL_NAME(var)));
  }
}

/* caller */
{
  if (code == GIMPLE_BIND)
  {
    printf("BIND {\n");
    print_bind_vars(gimple_bind_vars(stmt));
    printf("};\n");
  }
}
