/* Layout of filesec_t struct, solely for debugging. */

#include <sys/types.h>
#include <uuid/uuid.h>

struct _filesec {
	int	fs_valid;
#define FS_VALID_UID		(1<<0)
#define FS_VALID_GID		(1<<1)
#define FS_VALID_UUID		(1<<2)
#define FS_VALID_GRPUUID	(1<<3)
#define	FS_VALID_MODE		(1<<4)
#define FS_VALID_ACL		(1<<5)
	uid_t	fs_uid;
	gid_t	fs_gid;
	uuid_t	fs_uuid;
	uuid_t	fs_grpuuid;
	mode_t	fs_mode;
	size_t	fs_aclsize;
	void	*fs_aclbuf;
};
