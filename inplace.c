#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define BLOCKSIZE (128 * 1024)

size_t modified = 0;
size_t unchanged = 0;
char srcbuf[BLOCKSIZE];
char dstbuf[BLOCKSIZE];

void print_stats() {
  printf("Changed bytes %ld (%ld blocks), unchanged %ld (%ld blocks), changed "
         "percent %lu\n",
         modified * BLOCKSIZE, modified, unchanged * BLOCKSIZE, unchanged,
         modified * 100 / (modified + unchanged));
}

void fail_with_perror(const char *message) {
  print_stats();
  perror(message);
  exit(1);
}

void fail_with_message(const char *message) {
  print_stats();
  fprintf(stderr, "%s\n", message);
  exit(1);
}

int readfully(int fd, char *buf, size_t size, const char *message) {
  size_t rem = size;

  while (rem > 0) {
    ssize_t rd = read(fd, buf, rem);
    if (rd == 0) {
      break;
    }

    if (rd <= 0) {
      fail_with_perror(message);
    }
    buf += rd;
    rem -= rd;
  }
  return size - rem;
}

void compare_and_write(int fd, char *buf, size_t size) {
  int rd = readfully(fd, dstbuf, size, "Failed to read destination");

  if (rd != size) {
    fail_with_message("Sizes don't match");
  }

  bool match = true;
  for (size_t i = 0; i < size; i++) {
    if (srcbuf[i] != dstbuf[i]) {
      match = false;
    }
  }

  if (match) {
    unchanged++;
    return;
  }
  modified++;

  int res = lseek(fd, -BLOCKSIZE, SEEK_CUR);
  if (res == (off_t)-1) {
    fail_with_perror("Failed to seek destination");
  }
  res = write(fd, srcbuf, size);
  if (res == -1) {
    fail_with_perror("Failed to write destination");
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr,
            "Usage: %s src_file dest_file\n"
            "The destination should already exist and be the same size "
            "as source.\n",
            argv[0]);
    exit(1);
  }

  int src = open(argv[1], O_RDONLY);
  if (src == -1) {
    perror("Failed to open src");
    exit(1);
  }

  int dest = open(argv[2], O_RDWR);
  if (dest == -1) {
    perror("Failed to open dest");
    exit(1);
  }

  while (true) {
    ssize_t rd = readfully(src, srcbuf, sizeof(srcbuf), "Failed to read stdin");

    if (rd == 0) {
      print_stats();
      exit(0);
    }
    compare_and_write(dest, srcbuf, rd);
  }
}
