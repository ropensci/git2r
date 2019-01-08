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

static void print_commit (git_commit *commit);
static int match_with_parent (git_commit *commit, int i,
			      git_diff_options *opts);

int main(int argc, char *argv[]) {
  int i, parents, unmatched;
  int count   = 0;
  int printed = 0;
  git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;
  git_oid oid;
  git_repository *repo = NULL;
  git_revwalk *walker  = NULL;
  git_commit *commit   = NULL;
  git_pathspec *ps     = NULL;
  git_tree *tree;
	
  git_libgit2_init();
  
  // Parse arguments and set up revwalker.
  diffopts.pathspec.strings = &argv[1];
  diffopts.pathspec.count   = 1;
  git_pathspec_new(&ps, &diffopts.pathspec);

  // Open the repository.
  git_repository_open_ext(&repo, ".", 0, NULL);
  git_revwalk_new(&walker,repo);
  git_revwalk_sorting(walker,GIT_SORT_TIME);
  git_revwalk_push_head(walker);
  
  // Use the revwalker to traverse the history.
  for (; !git_revwalk_next(&oid, walker); git_commit_free(commit)) {
    git_commit_lookup(&commit, repo, &oid);
    
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
    
    print_commit(commit);
  }
  
  git_pathspec_free(ps);
  git_revwalk_free(walker);
  git_repository_free(repo);
  git_libgit2_shutdown();
  
  return 0;
}


/** Helper to print a commit object. */
static void print_commit (git_commit *commit) {
  char buf[GIT_OID_HEXSZ + 1];
  int i, count;
  const git_signature *sig;
  const char *scan, *eol;
  
  git_oid_tostr(buf, sizeof(buf), git_commit_id(commit));
  printf("%s\n", buf);
}

/** Helper to find how many files in a commit changed from its nth parent. */
static int match_with_parent (git_commit *commit, int i,
			      git_diff_options *opts) {
  git_commit *parent;
  git_tree *a, *b;
  git_diff *diff;
  int ndeltas;
  
  git_commit_parent(&parent, commit, (size_t) i);
  git_commit_tree(&a, parent);
  git_commit_tree(&b, commit);
  git_diff_tree_to_tree(&diff, git_commit_owner(commit), a, b, opts);
  
  ndeltas = (int) git_diff_num_deltas(diff);
  
  git_diff_free(diff);
  git_tree_free(a);
  git_tree_free(b);
  git_commit_free(parent);
  
  return ndeltas > 0;
}
