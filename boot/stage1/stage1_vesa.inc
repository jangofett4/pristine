; Pristine
; stage1_vesa: VESA VBE subroutines
; SPDX-License-Identifier: MIT

VESA_VBE_INFO_BUF equ 0x7000
VESA_VBE_MODE_BUF equ 0x7100

struc VbeInfoBlock
    .VbeSignature:      resb 4   ; 'VESA'
    .VbeVersion:        resw 1   ; 0x0100, 0x0200, 0x0300
    .OemStringPtr:      resd 1   ; OEM string pointer
    .Capabilities:      resd 1   ; controller capabilities
    .VideoModePtr:      resd 1   ; supported video modes pointer
    .TotalMemory:       resw 1   ; total memory (x 64KiB)
    .OemSoftwareRev:    resw 1   ; VBE software revision
    .OemVendorNamePtr:  resd 1   ; vendor name string pointer
    .OemProductNamePtr: resd 1   ; product name string pointer
    .OemProductRevPtr:  resd 1   ; product revision string pointer
    .Reserved:          resb 222 ; reserved
endstruc

struc VbeModeInfoBlock
    .ModeAttributes:        resw 1 ; mode attributes
    .WinAAttributes:        resb 1 ; window A attributes
    .WinBAttributes:        resb 1 ; window B attributes
    .WinGranularity:        resw 1 ; window granularity
    .WinSize:               resw 1 ; window size
    .WinASegment:           resw 1 ; window A start segment
    .WinBSegment:           resw 1 ; window B start segment
    .WinFuncPtr:            resd 1 ; real mode pointer to window function
    .BytesPerScanLine:      resw 1 ; bytes per scan line
    .XResolution:           resw 1 ; horizontal resolution
    .YResolution:           resw 1 ; vertical resolution
    .XCharSize:             resb 1 ; character cell width in pixels
    .YCharSize:             resb 1 ; character cell height in pixels
    .NumberOfPlanes:        resb 1 ; number of memory planes
    .BitsPerPixel:          resb 1 ; bits per pixel
    .NumberOfBanks:         resb 1 ; number of banks
    .MemoryModel:           resb 1 ; memory model type
    .BankSize:              resb 1 ; bank size
    .NumberOfImagePages:    resb 1 ; number of images
    .Reserved1:             resb 1 ; reserved
    .RedMaskSize:           resb 1 ; size of direct color red mask in bits
    .RedFieldPosition:      resb 1 ; bit position of LSB of red mask
    .GreenMaskSize:         resb 1 ; size of direct color green mask in bits
    .GreenFieldPosition:    resb 1 ; bit position of LSB of green mask
    .BlueMaskSize:          resb 1 ; size of direct color blue mask in bits
    .BlueFieldPosition:     resb 1 ; bit position of LSB of blue mask
    .RsvdMaskSize:          resb 1 ; size of direct color reserved mask bits
    .RsvdFieldPosition:     resb 1 ; bit position of LSB of reserved mask
    .DirectColorModeInfo:   resb 1 ; direct color mode attributes

    ; VBE 2.0
    .PhysBasePtr:           resd 1 ; physical address for flat frame buffer
    .Reserved2:             resd 1 ; reserved
    .Reserved3:             resw 1 ; reserved

    ; VBE 3.0
    .LinBytesPerScanLine:   resw 1 ; bytes per scan line for linear modes
    .BnkNumberOfImagePages: resb 1 ; number of images for banked modes
    .LinNumberOfImagePages: resb 1 ; number of images for linear modes
    .LinRedMaskSize:        resb 1 ; size of red mask
    .LinRedFieldPosition:   resb 1 ; nit position of LSB of red
    .LinGreenMaskSize:      resb 1 ; size of green mask
    .LinGreenFieldPosition: resb 1 ; bit position of LSB of green
    .LinBlueMaskSize:       resb 1 ; size of blue mask
    .LinBlueFieldPosition:  resb 1 ; bit position of LSB of blue
    .LinRsvdMaskSize:       resb 1 ; size of reserved mask
    .LinRsvdFieldPosition:  resb 1 ; bit position of LSB of reserved
    .MaxPixelClock:         resd 1 ; maximum pixel clock

    .Reserved4:             resb 189
endstruc

stage1_vesa_get_info:
    xor ax, ax
    mov es, ax
    mov ah, 0x4f
    mov al, 0x00
    mov di, VESA_VBE_INFO_BUF
    mov byte [es:di + 0], 'V'
    mov byte [es:di + 1], 'B'
    mov byte [es:di + 2], 'E'
    mov byte [es:di + 3], '2'
    int 0x10
    ret

; Input: CX = mode number to query
stage1_vesa_get_mode_info:
    xor ax, ax
    mov es, ax
    mov ah, 0x4f
    mov al, 0x01
    mov di, VESA_VBE_MODE_BUF
    int 0x10
    ret

; Input: CX = mode to switch
stage1_vesa_switch_mode:
    mov ah, 0x4f
    mov al, 0x02
    int 0x10
    ret
    