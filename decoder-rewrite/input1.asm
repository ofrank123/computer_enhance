;;; Exercise 02 - Input 2
	bits 16

	mov si, bx
	mov dh, al
	mov cl, 12
	mov ch, -12
	mov cx, 12
	mov cx, -12
	mov dx, 3948
	mov dx, -3948
	mov al, [bx + si]
	mov bx, [bp + di]
	mov dx, [bp]
	mov ah, [bx + si + 4]
	mov al, [bx + si + 4999]
	mov [bx + di], cx
	mov [bp + si], cl
	mov [bp], ch
	mov ax, [bx + di - 37]
	mov [si - 300], cx
	mov dx, [bx - 32]
	mov [bp + di], byte 7
	mov [di + 901], word 347
	mov bp, [5]
	mov bx, [3458]
	mov ax, [2555]
	mov ax, [16]
	mov [2554], ax
	mov [15], ax

	add bx, [bx + si]
	add bx, [bp + 0]
	add si, word 2
	add bp, word 2
	add cx, word 8
	add bx, [bp + 0]
	add cx, [bx + 2]
	add bh, [bp + si + 4]
	add di, [bp + di + 6]
	add [bx + si], bx
	add [bp + 0], bx
	add [bp + 0], bx
	add [bx + 2], cx
	add [bp + si + 4], bh
	add [bp + di + 6], di
	add [bx], byte 34
	add [bp + si + 1000], word 29
	add ax, [bp + 0]
	add al, [bx + si]
	add ax, bx
	add al, ah
	add ax, word 1000
	add al, byte 226
	add al, byte 9

	sub bx, [bx + si]
	sub bx, [bp + 0]
	sub si, word 2
	sub bp, word 2
	sub cx, word 8
	sub bx, [bp + 0]
	sub cx, [bx + 2]
	sub bh, [bp + si + 4]
	sub di, [bp + di + 6]
	sub [bx + si], bx
	sub [bp + 0], bx
	sub [bp + 0], bx
	sub [bx + 2], cx
	sub [bp + si + 4], bh
	sub [bp + di + 6], di
	sub [bx], byte 34
	sub [bx + di], word 29
	sub ax, [bp + 0]
	sub al, [bx + si]
	sub ax, bx
	sub al, ah
	sub ax, word 1000
	sub al, byte 226
	sub al, byte 9

	cmp bx, [bx + si]
	cmp bx, [bp + 0]
	cmp si, word 2
	cmp bp, word 2
	cmp cx, word 8
	cmp bx, [bp + 0]
	cmp cx, [bx + 2]
	cmp bh, [bp + si + 4]
	cmp di, [bp + di + 6]
	cmp [bx + si], bx
	cmp [bp + 0], bx
	cmp [bp + 0], bx
	cmp [bx + 2], cx
	cmp [bp + si + 4], bh
	cmp [bp + di + 6], di
	cmp [bx], byte 34
	cmp [4834], word 29
	cmp ax, [bp + 0]
	cmp al, [bx + si]
	cmp ax, bx
	cmp al, ah
	cmp ax, word 1000
	cmp al, byte 226
	cmp al, byte 9
