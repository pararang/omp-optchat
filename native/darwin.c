#include <node_api.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

// Let the C compiler handle Darwin's variadic openat/fcntl ABI. Calling these
// directly through fixed-signature FFI is incorrect on Apple Silicon.
static napi_value system_error(napi_env env, int number, const char *operation) {
  const char *code;
  switch (number) {
    case EACCES: code = "EACCES"; break;
    case EBADF: code = "EBADF"; break;
    case EEXIST: code = "EEXIST"; break;
    case EINTR: code = "EINTR"; break;
    case EINVAL: code = "EINVAL"; break;
    case EIO: code = "EIO"; break;
    case ELOOP: code = "ELOOP"; break;
    case EMFILE: code = "EMFILE"; break;
    case ENAMETOOLONG: code = "ENAMETOOLONG"; break;
    case ENFILE: code = "ENFILE"; break;
    case ENOENT: code = "ENOENT"; break;
    case ENOSPC: code = "ENOSPC"; break;
    case ENOTDIR: code = "ENOTDIR"; break;
    case EPERM: code = "EPERM"; break;
    case EROFS: code = "EROFS"; break;
    case EWOULDBLOCK: code = "EWOULDBLOCK"; break;
    default: code = "ERR_SYSTEM_ERROR"; break;
  }
  napi_value error, message, name, syscall, value;
  napi_create_string_utf8(env, strerror(number), NAPI_AUTO_LENGTH, &message);
  napi_create_string_utf8(env, code, NAPI_AUTO_LENGTH, &name);
  napi_create_error(env, name, message, &error);
  napi_create_string_utf8(env, operation, NAPI_AUTO_LENGTH, &syscall);
  napi_set_named_property(env, error, "syscall", syscall);
  napi_create_int32(env, number, &value);
  napi_set_named_property(env, error, "errno", value);
  napi_throw(env, error);
  return NULL;
}

static bool arguments(napi_env env, napi_callback_info info, size_t count, napi_value *values) {
  size_t actual = count;
  if (napi_get_cb_info(env, info, &actual, values, NULL, NULL) != napi_ok || actual != count) {
    napi_throw_type_error(env, NULL, "Invalid native storage arguments");
    return false;
  }
  return true;
}

static bool integer(napi_env env, napi_value value, int *result) {
  int32_t number;
  if (napi_get_value_int32(env, value, &number) != napi_ok) {
    napi_throw_type_error(env, NULL, "Native storage descriptor or flags must be a number");
    return false;
  }
  *result = number;
  return true;
}

static bool filename(napi_env env, napi_value value, char *name) {
  size_t length;
  if (napi_get_value_string_utf8(env, value, NULL, 0, &length) != napi_ok) {
    napi_throw_type_error(env, NULL, "Native storage name must be a string");
    return false;
  }
  if (length >= PATH_MAX) {
    system_error(env, ENAMETOOLONG, "name");
    return false;
  }
  if (napi_get_value_string_utf8(env, value, name, PATH_MAX, &length) != napi_ok) return false;
  if (memchr(name, '\0', length) != NULL) {
    napi_throw_type_error(env, NULL, "Native storage name contains a null byte");
    return false;
  }
  return true;
}

static napi_value open_at(napi_env env, napi_callback_info info) {
  napi_value values[4], result;
  int directory, flags, mode;
  char name[PATH_MAX];
  if (!arguments(env, info, 4, values) || !integer(env, values[0], &directory) ||
      !filename(env, values[1], name) || !integer(env, values[2], &flags) ||
      !integer(env, values[3], &mode)) return NULL;
  int fd = openat(directory, name, flags | O_CLOEXEC, (mode_t)mode);
  if (fd == -1) return system_error(env, errno, "openat");
  if (napi_create_int32(env, fd, &result) != napi_ok) {
    close(fd);
    return NULL;
  }
  return result;
}

static napi_value mkdir_at(napi_env env, napi_callback_info info) {
  napi_value values[2], result;
  int directory;
  char name[PATH_MAX];
  if (!arguments(env, info, 2, values) || !integer(env, values[0], &directory) ||
      !filename(env, values[1], name)) return NULL;
  if (mkdirat(directory, name, 0700) == -1) return system_error(env, errno, "mkdirat");
  napi_get_undefined(env, &result);
  return result;
}

static napi_value rename_at(napi_env env, napi_callback_info info) {
  napi_value values[4], result;
  int old_directory, new_directory;
  char old_name[PATH_MAX], new_name[PATH_MAX];
  if (!arguments(env, info, 4, values) || !integer(env, values[0], &old_directory) ||
      !filename(env, values[1], old_name) || !integer(env, values[2], &new_directory) ||
      !filename(env, values[3], new_name)) return NULL;
  if (renameat(old_directory, old_name, new_directory, new_name) == -1)
    return system_error(env, errno, "renameat");
  napi_get_undefined(env, &result);
  return result;
}

static napi_value unlink_at(napi_env env, napi_callback_info info) {
  napi_value values[2], result;
  int directory;
  char name[PATH_MAX];
  if (!arguments(env, info, 2, values) || !integer(env, values[0], &directory) ||
      !filename(env, values[1], name)) return NULL;
  if (unlinkat(directory, name, 0) == -1) return system_error(env, errno, "unlinkat");
  napi_get_undefined(env, &result);
  return result;
}

static napi_value read_directory(napi_env env, napi_callback_info info) {
  napi_value value, result;
  int directory;
  if (!arguments(env, info, 1, &value) || !integer(env, value, &directory)) return NULL;
  // A dup would share the directory cursor with the held descriptor. Open "."
  // relative to it instead, so every replay starts with an independent cursor.
  int fd = openat(directory, ".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
  if (fd == -1) return system_error(env, errno, "openat");
  DIR *stream = fdopendir(fd);
  if (stream == NULL) {
    int number = errno;
    close(fd);
    return system_error(env, number, "fdopendir");
  }
  if (napi_create_array(env, &result) != napi_ok) {
    closedir(stream);
    return NULL;
  }
  uint32_t index = 0;
  for (;;) {
    errno = 0;
    struct dirent *entry = readdir(stream);
    if (entry == NULL) {
      int number = errno;
      int closed = closedir(stream);
      if (number != 0) return system_error(env, number, "readdir");
      if (closed == -1) return system_error(env, errno, "closedir");
      return result;
    }
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
    napi_value name;
    if (napi_create_string_utf8(env, entry->d_name, entry->d_namlen, &name) != napi_ok ||
        napi_set_element(env, result, index++, name) != napi_ok) {
      closedir(stream);
      return NULL;
    }
  }
}

static napi_value descriptor_path(napi_env env, napi_callback_info info) {
  napi_value value, result;
  int fd;
  char path[PATH_MAX];
  if (!arguments(env, info, 1, &value) || !integer(env, value, &fd)) return NULL;
  if (fcntl(fd, F_GETPATH, path) == -1) return system_error(env, errno, "fcntl(F_GETPATH)");
  napi_create_string_utf8(env, path, NAPI_AUTO_LENGTH, &result);
  return result;
}

static napi_value lock_directory(napi_env env, napi_callback_info info) {
  napi_value value, result;
  int fd;
  if (!arguments(env, info, 1, &value) || !integer(env, value, &fd)) return NULL;
  if (flock(fd, LOCK_EX | LOCK_NB) == -1) return system_error(env, errno, "flock");
  // Ownership belongs to this open file description, released by close/process
  // death. Independent opens, including same-process ones, cannot take it over.
  napi_get_undefined(env, &result);
  return result;
}

static napi_value initialize(napi_env env, napi_value exports) {
  const napi_property_descriptor properties[] = {
    {"openAt", NULL, open_at, NULL, NULL, NULL, napi_default, NULL},
    {"mkdirAt", NULL, mkdir_at, NULL, NULL, NULL, napi_default, NULL},
    {"renameAt", NULL, rename_at, NULL, NULL, NULL, napi_default, NULL},
    {"unlinkAt", NULL, unlink_at, NULL, NULL, NULL, napi_default, NULL},
    {"readDirectory", NULL, read_directory, NULL, NULL, NULL, napi_default, NULL},
    {"descriptorPath", NULL, descriptor_path, NULL, NULL, NULL, napi_default, NULL},
    {"lockDirectory", NULL, lock_directory, NULL, NULL, NULL, napi_default, NULL},
  };
  napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, initialize)
