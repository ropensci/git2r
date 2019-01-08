/*
 * libgit2 "log" example - shows how to walk history and get commit info
 *
 * Written by the libgit2 contributors
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide. This software is distributed without any warranty.
 *
 * You should have received a copy of the CC0 Public Domain Dedication along
 * with this software. If not, see
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include "common.h"

/**
 * This example demonstrates the libgit2 rev walker APIs to roughly
 * simulate the output of `git log` and a few of command line arguments.
 * `git log` has many many options and this only shows a few of them.
 *
 * This does not have:
 *
 * - Robust error handling
 * - Colorized or paginated output formatting
 * - Most of the `git log` options
 *
 * This does have:
 *
 * - Examples of translating command line arguments to equivalent libgit2
 *   revwalker configuration calls
 * - Simplified options to apply pathspec limits and to show basic diffs
 */

/** log_state represents walker being configured while handling options */
struct log_state {
  git_repository *repo;
  const char *repodir;
  git_revwalk *walker;
  int hide;
  int sorting;
  int revisions;
};

/** utility functions that are called to configure the walker */
static void push_rev(struct log_state *s, git_object *obj, int hide);

/** log_options holds other command line options that affect log output */
struct log_options {
  int show_log_size;
  int skip, limit;
  int min_parents, max_parents;
  git_time_t before;
  git_time_t after;
  const char *author;
  const char *committer;
  const char *grep;
};

/** utility functions that parse options and help with log output */
static void init_options(struct log_state *s, struct log_options *opt);
static void print_commit(git_commit *commit, struct log_options *opts);
static int match_with_parent(git_commit *commit, int i, git_diff_options *);

int main(int argc, char *argv[]) {
  int i, unmatched;
  int count   = 0;
  int printed = 0;
  int parents = 0;
  struct log_state s;
  struct log_options opt;
  git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;
  git_oid oid;
  git_commit *commit = NULL;
  git_pathspec *ps = NULL;
  git_tree *tree;
	
  git_libgit2_init();
  
  // Parse arguments and set up revwalker.
  init_options(&s,&opt);
  diffopts.pathspec.strings = &argv[1];
  diffopts.pathspec.count   = 1;
  git_pathspec_new(&ps, &diffopts.pathspec);

  // Open the repository.
  git_repository_open_ext(&s.repo, ".", 0, NULL);
  push_rev(&s, NULL, 0);
  
  // Use the revwalker to traverse the history.
  for (; !git_revwalk_next(&oid, s.walker); git_commit_free(commit)) {
    git_commit_lookup(&commit, s.repo, &oid);
    
    parents = (int) git_commit_parentcount(commit);
    unmatched = parents;
      
    if (parents == 0) {
      git_commit_tree(&tree, commit);
      if (git_pathspec_match_tree(NULL,tree,GIT_PATHSPEC_NO_MATCH_ERROR,ps)
	  != 0)
	unmatched = 1;
	git_tree_free(tree);
      } else if (parents == 1) {
	unmatched = match_with_parent(commit, 0, &diffopts) ? 0 : 1;
      } else {
	for (i = 0; i < parents; ++i) {
	  if (match_with_parent(commit, i, &diffopts))
	    unmatched--;
	}
      }
      
    if (unmatched > 0)
      continue;
    
    print_commit(commit, &opt);
  }
  
  git_pathspec_free(ps);
  git_revwalk_free(s.walker);
  git_repository_free(s.repo);
  git_libgit2_shutdown();
  
  return 0;
}

/** Push object (for hide or show) onto revwalker. */
static void push_rev(struct log_state *s, git_object *obj, int hide) {
  hide = s->hide ^ hide;
  
  /** Create revwalker on demand if it doesn't already exist. */
  if (!s->walker) {
    git_revwalk_new(&s->walker, s->repo);
    git_revwalk_sorting(s->walker, s->sorting);
  }
  
  if (!obj)
    git_revwalk_push_head(s->walker);
  else
    git_revwalk_push(s->walker, git_object_id(obj));
  
  git_object_free(obj);
}

/** Helper to print a commit object. */
static void print_commit(git_commit *commit, struct log_options *opts) {
  char buf[GIT_OID_HEXSZ + 1];
  int i, count;
  const git_signature *sig;
  const char *scan, *eol;
  
  git_oid_tostr(buf, sizeof(buf), git_commit_id(commit));
  printf("%s\n", buf);
}

/** Helper to find how many files in a commit changed from its nth parent. */
static int match_with_parent(git_commit *commit, int i,
			     git_diff_options *opts) {
  git_commit *parent;
  git_tree *a, *b;
  git_diff *diff;
  int ndeltas;
  
  git_commit_parent(&parent, commit, (size_t) i);
  git_commit_tree(&a, parent);
  git_commit_tree(&b, commit);
  git_diff_tree_to_tree(&diff, git_commit_owner(commit), a, b, opts);
  
  ndeltas = (int)git_diff_num_deltas(diff);
  
  git_diff_free(diff);
  git_tree_free(a);
  git_tree_free(b);
  git_commit_free(parent);
  
  return ndeltas > 0;
}

/** Initialize the log options. */
static void init_options(struct log_state *s, struct log_options *opt) {
  
  memset(s, 0, sizeof(*s));
  s->sorting = GIT_SORT_TIME;
  
  memset(opt, 0, sizeof(*opt));
  opt->max_parents = -1;
  opt->limit = -1;
}
