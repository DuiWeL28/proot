/** Checks permissions on every component of path. Up to the location specifed
 *  by rel_path. If type is specified to be "read", it checks only execute 
 *  permissions. If type is specified to be "write, it makes sure that the 
 *  parent directory of the file specified by path also has write permissions.
 *  The permission check uses guest paths only.
 */

#include <linux/limits.h>
#include <string.h>
#include <errno.h>

#include "path/path.h"
#include "shared_structs.h"
#include "tracee/reg.h"


int check_dir_perms(Tracee *tracee, char type, char path[PATH_MAX], 
    char rel_path[PATH_MAX], Config *config)
{
    int status, perms;
    char meta_path[PATH_MAX];
    char shorten_path[PATH_MAX];
    int x = 1; 
    int w = 2;

    get_dir_path(path, shorten_path);
    status = get_meta_path(shorten_path, meta_path); 
    if(status < 0)
        return status;

    perms = get_permissions(meta_path, config, 0);

    if(type == 'w' && (perms & w) != w) 
        return -EACCES;
    
    if(type == 'r' && (perms & x) != x) 
        return -EACCES;

    while(strcmp(shorten_path, rel_path) != 0 && strlen(rel_path) < strlen(shorten_path)) {
        get_dir_path(shorten_path, shorten_path);
        if(!belongs_to_guestfs(tracee, shorten_path))
            break;

        status = get_meta_path(shorten_path, meta_path);
        if(status < 0)
            return status;

        perms = get_permissions(meta_path, config, 0);
        if((perms & x) != x) 
            return -EACCES;
        
    }

    return 0;
}

/** Gets a path without its final component.
 */

int get_dir_path(char path[PATH_MAX], char dir_path[PATH_MAX])
{
    int offset;

    strcpy(dir_path, path);
    offset = strlen(dir_path) - 1;
    if (offset > 0) {
        /* Skip trailing path separators. */
        while (offset > 1 && dir_path[offset] == '/')
            offset--;

        /* Search for the previous path separator. */
        while (offset > 1 && dir_path[offset] != '/')
            offset--;

        /* Cut the end of the string before the last component. */
        dir_path[offset] = '\0';
    }
    return 0;
}
