#pragma once
// Common stuff for this course

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define PANIC(...) fprintf(stderr, "Uh Oh! %s:%d: ", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

static struct StringArena {
	char *base;
	char *head;
	u32 size;
} strArena{};

static void initStrArena() {
	strArena.base = (char *) malloc(sizeof(char) * 64 * 1024);
	strArena.head = strArena.base;
	strArena.size = 64 * 1024;
}

static void destroyStrArena() {
	free(strArena.base);
}

static char *allocStr(u32 size) {
	assert(((u32) (strArena.head - strArena.base)) + size <= strArena.size);
	char *str = strArena.head;
	strArena.head += size;
	return str;
}
