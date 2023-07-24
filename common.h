#pragma once
// Common stuff for this course

#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct StringArena {
	char *base;
	char *head;
	u32 size;
} strArena{};

void initStrArena() {
	strArena.base = (char *) malloc(sizeof(char) * 64 * 1024);
	strArena.head = strArena.base;
	strArena.size = 64 * 1024;
}

void destroyStrArena() {
	free(strArena.base);
}

char *allocStr(u32 size) {
	assert(((u32) (strArena.head - strArena.base)) + size <= strArena.size);
	char *str = strArena.head;
	strArena.head += size;
	return str;
}

void expandCurrentStr(u32 size) {
	assert(((u32) (strArena.head - strArena.base)) + size <= strArena.size);
	strArena.head += size;
}
