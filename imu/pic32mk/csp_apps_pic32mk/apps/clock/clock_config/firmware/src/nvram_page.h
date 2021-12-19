/* 
 * File:   nvram_page.h
 * Author: root
 *
 * Created on October 31, 2020, 7:47 AM
 */

#ifndef NVRAM_PAGE_H
#define	NVRAM_PAGE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define NVM_STARTVADDRESS	0x9d008000  // virtual address
#define NVM_STARTPADDRESS	0x1d008000  // physical address


	extern const volatile uint32_t myflash[4096] __attribute__((section("myflash"), space(prog), address(NVM_STARTVADDRESS)));
	extern uint32_t *pmyflash;


#ifdef	__cplusplus
}
#endif

#endif	/* NVRAM_PAGE_H */

