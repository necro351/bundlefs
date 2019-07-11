TL;DR
=====
1) Change into core
2) Run make check
3) All tests should pass
4) Change into fuse
5) We need a Makefile and fuse should use core so we can do some prototyping
   and system testing

Abstract
========

BundleFS is a stacking file system that exposes a transactional git-like file
system interface to an underlying file system that needs to be tracked. The
underlying file system need not support snapshots (writable or read-only),
transactions, layering, unioning, or any other advanced features (e.g., Ext4),
but by wrapping the filesystem with BundleFS, these features become available.
Although writable snapshots and transactions are useful on their own, BundleFS
uses these mechanisms as building blocks to implement support for bundles,
which are self-contained directories of backup files that can be created with a
git-diff like interface and can be used like patches to, e.g., provide
incremental changes to a common file system state, store backups on a backup
disk, consistently migrate file system state from one host to another, or to
merge and resolve conflicts with a larger encompassing distributed file system
state.

Intro
=====

Transactions in operating and distributed systems is still a largely open
problem but even so isolation for complex processes has already come. VMs
provide one level of isolation for entire operating systems while containers
provide isolation for groups of processes within an operating system. Although
mechanisms for achieving isolation are well supported and rapidly expanding in
choice of level of granularity how work is performed, or how a commonly
accepted data set is moved from one state forward to another, is more
application specific. It is perfectly reasonable for a set of containers to
modify a shared Cassandra database for example, but there is also a converse
need for a more general approach that can be adopted with less development
time, even if it means potentially worse performance to an application-specific
optimized approach. A distributed file system would be such a general approach
but how does one corral many different types of file systems, laptops,
container volumes, shared networked volumes, each with different rates of
churn, and manage all the conflicts that may arise as they all update
potentially the same file? The common solution is partitioning for scale, and
late resolution of conflicts for conflict management, i.e., something like git.
However git does not handle large files well, and does not provide a file
system interface. BundleFS addresses these issues and is designed explicitly to
work out of the box with older local file systems while providing a larger
distributed namespace that local file systems can remain oblivious to even as
they participate in making changes to it. A good simplifying similie for
BundleFS is that it allows small to large groups of people to track changes to
all files as if they were in a large shared source control repository with
commensurate features (provenance tracking, revision control, backup and
restore, distribution of small incremental updates to shared files). It may
also be helpful for understanding to point out one of the things BundleFS does
_not_ do. It does not attempt to impose immediate and global consistency (e.g.,
two or more users can freely modify the same file, just as in git, and will
have to resolve the conflict the next time they directly or transitively sync
with a common ancestor state, just as in git). In general BundleFS tries to
make it very easy to understand, search, access, and manage the history of an
organization's files across all its various file systems used by its users. It
does _not_ attempt to create the illusion of a massive, single shared file
system where all changes are immediately visible to all parties no matter where
or with what quality of network connectivity they are made.

Design
======

There are three components to BundleFS:

(1) The stacking file system (which for performance reasons is an in-kernel
stacking FS like eCryptFS based on wrapFS, not a FUSE FS, though it could be
ported to FUSE relatively easily). This is called BundleFS.

(2) The revision control tools which are user-level tools that understand the
bundle format used by BundleFS and can merge together and partition revision
control graphs serialized in bundles created by BundleFS. The revision control
tools are used to view and manage the combination of all bundles contributed by
all users in an organization as they commit their changes to their local
bundles and potentially overlapping files modified in other bundles as well.

(3) The bundle on-disk format which is a simple archival format that can be
understood by the standard command line tools and easily backed up, restored,
transferred, and compressed by existing industry-grade storage solutions, or
simpler solutions, e.g., rsync.

BundleFS focuses by prioritizing features in this order:

(1) Data protection: The stacking FS chooses to use more traditional
undo-logging of files so that changes to BundleFS are almost always realized as
changes to the underlying file system. This means that if BundleFS is
unmounted, the underlying FS represents the current working HEAD state. This is
intentional and represents BundleFS being committed to a policy of
non-interference and keeping users as close to a working FS state as possible
at all times if BundleFS is taken out of the equation. In the same vein, all of
BundleFS's state is kept in a separate directory that can be unlinked,
archived, or even hosted on a different partition or underlying FS. This policy
of non-interference attempts to place the majority of the burden of data
protection on the underlying FS implementation, which typically makes sense and
is how BundleFS achieves most of its data protection.

(2) Transparency: BundleFS's bundle format is easily understood by existing
command line tools and new tools can be easily written to understand it as
well. Furthermore since the bundles are just directories of files they can be
moved around, checksummed, compressed, and distributed. The on-disk bundle
format is always preserved and more efficient approaches are abandoned if they
comprimise the simplicity of the bundle format.

(3) Conflict Resolution: As much as possible conflict resolution is made
transparent and handled out of path. Out of path because kernel code run by
mmap is the wrong place to ask users how to handle conflicts in files.
Transparent so that it is straight-forward to add application support for
better support of conflict management, and ultimately, conflict management
itself can be supported in more robust, perhaps even proprietary tools that
make it easy for organizations to minimize, handle, and intelligently
distribute conflict resolution.

(4) Performance: When possible performance is achieved (e.g., by using a
stackable FS instead of FUSE) but never at the expense of the above features.
For example when making changes to a file shared by another branch the
block-size used for faulting pieces is 16MB so that the bundle format can use
the directory/namespace structure of the underlying FS to index modified files
and the indexes are visible to command line tools exploring the bundle format,
even though the large block size means that many workloads will end up making
complete copies of the shared snapshot or incurr large latencies. There are
much more efficient ways of supporting faulting that would allow for very small
block sizes, but these would require a more efficient and integrated indexing
approach that would preclude simply using the underlying FS's
directory/namespace structure and would require special command-line tools to
browse the bundles.

Bundle Format
-------------

A bundle is a set of commits and an accompanying set of aliases (tags) that
point to one or more of those commits. A bundle consists of at least a HEAD tag
and then perhaps more tags beyond that. Like in git, a commit is comprised of a
tree as well as a set of parent and a set of child commits. A tree is a
hierarchical structure that is comprised of one or more absolute path to object
or stub mappings. An object is a collection of faulted pieces or piece stubs. A
stub is a reference to data that may not be accessible locally and a hash is
provided in its place so the proper Merkel-ification can be computed
regardless. Stubs can be within a set or can be individually packed. For
example a tree may use a stub to refer to several entries within a directory,
these would be individually packed so that when (if) the stub is expanded the
new items contribute to the set of items in the directory rather than creating
a new sub-directory.

All sets of items are represented as directories of files in the underlying
file system and all references are represented as simple files with the address
to follow and the method by which to follow encoded as plain text in the simple
file.

Example Bundle:
Manifest:
bundles/i0/pieces
bundles/i0/pieces/i0
bundles/i0/pieces/i1
bundles/i0/pieces/i2
bundles/i0/pieces/i3
bundles/i0/files/i0
bundles/i0/trees/i0/home/rick/hello.tar.gz
bundles/i0/commit
bundles/i0/commit/message
bundles/i0/commit/root
bundles/i0/commit/parents

The 0th bundle only has one tree: bundles/0/trees/0. Since trees can contain
the manifest of an arbitrary file-tree hierarchy only one tree is necessary in
many cases. Since there is only one tree (in this simple example), the root
must point to it:

bundles/i0/commit/root:
0

The tree tells us what object each path maps to in this binary format:

bundles/i0/trees/i0/home/rick/hello.tar.gz:
{type: file, value: i0}

In this case the file is quite large at a little under 64MB, so it is broken
into four pieces. A file is represented with a file in the files sub-dir of the
bundle and it merely lists each piece's ID one after the other in binary
format. This is represented as a comma delimited list so we can read it.

bundles/i0/files/i0:
0,1,2,3

Finally each of the pieces themselves are 16MB or less of data that was copied
from the underlying file system. In this case every piece was modified in this
commit.

How BundleFS Creates Bundles
----------------------------

BundleFS does not use the snapshot metaphor but rather the version control
metaphore. Users do not modify snapshots but rather modify branches. Their
changes are staged as they are made and then are free to be asynchronously
copied into place after commit. BundleFS creates a bundle as soon as a branch
is created. Modifications to the branch are staged in the bundle. When the
modifications are committed the bundle is closed and a new one is created. For
simplicity we do not allow the user to commit an empty bundle.

To see how a bundle is constructed piece-by-piece let us walk through an
example. First the user creates a branch, this creates a mount point at
/var/bundlefs/mnt/0. The branch is a clone of some other committed file system
state. Next the user modifies /home/rick/hell.tar.gz by (let us suppose)
running a transform program on it that rewrites the file with a one-time-pad.
First the file is opened to perform the writes and this causes the tree file to
fault in an entry indicating this file was modified in this bundle.
Second the writes themselves are implemented by BundleFS by detecting that the
four pieces the file is comprised of have not ever been modified (it implicitly
knows this because the pieces have not already been faulted into the bundle) so
it will have to fault each piece in and apply the write. A piece is faulted in
by doing a read-modify-write: finding the piece the write modifies, copying it
into the bundle, and applying the write to the copy in the bundle. This has
performance implications that are ignored for simplicity at this point. It is
worth noting that with robust file caching most of the repeated copying from
doing faulting in this simple manner is done in memory, in the kernel, with
memcpy, and does not incur additional IO on top of a more sophisticated
log-structured small-block-size design. Once the write completes the bundle has
a tree file, the file pointing at the pieces, and each of the (modified)
pieces. The bundle is now almost completely formed, the final step will be when
the user (or backup daemon perhaps) commits the changes and a commit message
and list of parents will be created (in this case one parent only since this
change was not the result of a merge).

Merging in BundleFS
-------------------

Merging in BundleFS means creating a bundle that shows how to converge one or
more branches into a single branch. This is the primary mechanism by which
long-running transactions are ultimately committed. To see how merging is the
same as committing a transaction lets look at a simple example.

First we begin a transaction by cloning the branch we want to transactionally
make changes to (it could be the HEAD or some other branch a group of processes
are working on, e.g., a branch used by a container). This creates a private
(only we know about it) and writable branch. We make all the changes we want to
this branch and then commit them. At this point we have saved our desired
changes, but have not yet managed to publish them. To publish our changes we
must merge them back into the branch we want them to be visible from (e.g.,
HEAD or a shared branch used by a group of processes or container). This means
merging our private committed branch with the public branch. If there are no
conflicts then files we modified are orthogonal to files modified by other
transactions and the commit can proceed, otherwise a special conflict branch is
created where conflicting files are stored side-by-side and the user is
expected to remove one or the other (resolve conflicts) then mark the conflict
branch as resolved, at which point it can be committed. A committed merge
branch lists both the HEAD (or other branch) and the private branch as its
parents. It is important to note that for true transactional isolation files
are marked as read even if they are not modified, so that during a merge if one
branch modifies a file that was only read by the other, it will still be
registered as a conflict and must be explicitly acknowleged during conflict
resolution as being OK (or not). Alternatively if the branch cannot be merged
and the process performed is repeatable, the private branch can be discarded
and a new one can be formed by cloning the more recent HEAD. This is analagous
to a traditional transactional system forcing an abort on one of the processes.

Up/Downloading, Mounting, and Moving Bundles
--------------------------------------------

Bundles are a great way to package large changes made to a file system and can
be easiliy created by mounting BundleFS on top of a file system and making
changes to it. A bundle is 1:1 with a commit or state transition from one point
in the revision graph to the next. Containers and package management systems
work by mirroring a shallow tree of file system states that clients can run. It
would be sensible to represent the root state (perhaps a bare-bones ubuntu
image) as a root bundle (a single commit) and then represent children that
inherit from this image as subsequent bundles where each lists the root bundle
as its parent. Then a package distributer can host the base root bundle, and
then optional branch bundles that are based on the root. Each of these bundles
can be downloaded separately and then cloned and mounted.

If a bundle is going to be uploaded it makes sense to hash its contents and
compute the hash of the root of the Merkel tree. This process is delayed since
bundles are so much larger than source control repositories but it can be
performed. To Merkel-ify a bundle the hashes of all objects are computed and
are used as the identifier and the prefixing 'i' is stripped. The name of the
bundle becomes the hash of the root of the Merkel tree. Duplicate objects (if
there are any) are deduplicated. To download and mount a bundle merely unpack
the bundle in /var/bundlefs/bundles and a new directory named after the
bundle's SHA1 is created. At this point if the bundle's dependencies (in
parents) also exist the bundle can be treated like any other branch and can be
cloned and mounted.

Since bundles are just tar archives they can be mirrored, copied, hosted,
uploaded, downloaded, archived, and stored like any simple file.

Performance Optimizations
-------------------------

BundleFS is acceptable for many workloads (those that only modify smaller
files, create mostly new files, or sequentially append to existing files) but
for many other workloads (those that randomly update large files) the vast
majority of IO is spent on write amplification (due to the 16MB piece-size).
This can be potentially mitigated with a Bundle 2.0 spec where a smaller
block-size is supported (e.g., 4--16KB) and indexing is implemented using an
index stored in, e.g., LevelDB, instead of as simple names in directories.
Compaction would become a requirement as well at this point as randomly reading
4KB or 16KB can be much less efficient than 16MB pieces. These optimizations
drastically increase the complexity of BundleFS but only make some workloads
more efficient where as the much simple design above allows for all of
BundleFS's features to be used immediately. So we are simply stating that we
made a conscious decision to use a simple more rapidly implemented design while
leaving the door open to a more sophisticated on-disk format and file system
that can be more efficient for more random-access workloads.

Implementation
==============

BundleFS is implemented by forking wrapfs from Erez Zadok for Linux (and BSD).
Both BundleFS and the tests link against a common library in C that can run
both in user-space and the kernel. The user-space library is also linked with
the command-line tools that Merkel-ify a bundle so it can be shared with
others. The common library is implemented on top of a context that is either
implemented in terms of the underlying file system in the kernel or using the
POSIX API for unit tests and the command line tools.

The common library exposes three objects that can be manipulated with the
common library's API to browse, manage, modify, and create bundles:

(1) Pieces
(2) Files
(3) Trees
(4) Stubs
(5) Commits

Commits are 1:1 with bundles and contain all information necessary to modify
their parent commits to the state described in the commit message. Commits
exist in two states: (1) staging, and (2) committed. Commits are created based
on zero, one, or more parent commits in the staging state. At this point any
files within the commit that are modified must have the requisite trees created
and/or updated and files and pieces faulted in or created so they can then be
modified there.

Stubs are used to perform wide-scale changes across a very large distributed
repository. Stubs can replace other trees so that a user can download the
latest bundle for a very large repository without having to download all of the
contained trees, but only the one(s) in which he plans to make modifications. A
stub appears as an empty un-readable un-writable directory when the bundle is
cloned and mounted and it includes the SHA1 root of the Merkel tree rooted at
that stub so a correct Merkel root can be computed when the user commits
changes. This form of scaling requires that directories contain a number of
items that can be efficiently listed by the underlying file system on a single
node. It would be straight forward to support stubbing out sub-sets of
directory items but directory balancing is already a commonly used tool for
partitioning and scaling that this extra work is left for the future. Stubs are
just files in the tree that have the same name as the directory they are
stubbing out and their contents are the Merkel root of that stubbed tree.

Trees are directory trees that match up with the trees they are bundling. The
files are lists of piece IDs instead of actual file content and zero or more of
the directories may be stub files.

Files are simple files that represent actual files with data by indicating a
list of pieces that map the file offsets to actual data.

Pieces are 16MB or smaller immutable ranges of data that can be mapped to by
Files in Trees and are all stored together in a pieces directory of the bundle.

The API used by the common library is very much like the POSIX file system API:
* open:   open a file, adding it to the tree if necessary when the bundle is
          staging.
* close:  close a file.
* unlink: remove a file, removing it from the tree when staging.
* read:   read from a file, marking that piece as read from when staging.
* write:  write to a file, faulting in that piece or updating the already
          faulted in pieces when staging. 
* mkdir:  create a new directory, adding it to the tree when staging.
* rmdir:  remove a directory, removing it from the tree when staging.
* list:   list the contents of a directory.

There are additional API calls to support management and creation of bundles:
* format: format an empty directory so it can be used with BundleFS.
* init:   initialize the server (either FUSE or the kernel module).
* clone:  clone a commit, creating a new branch that can be mounted.
* mount:  mount a commit as a writable snapshot.
* umount: unmount a fully committed writable snapshot.
* merge:  merge one branch into another producing a new commit with two
	  parents. If there are conflicts the branch is left in the staging
	  state and conflicts are indicated. If none the branch is left in the
          committed state.

The common library is (system) tested using a FUSE file system that implements
the BundleFS features in user space and tests BundleFS using shell scripts and
standard file system tests. There are also unit tests.
