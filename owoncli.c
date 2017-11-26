#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DISP_MMODE 0x01
#define DISP_SOMETHING 0x02 // really this is just a placeholder

uint8_t OLs[] = { 0x2b, 0x3f, 0x30, 0x3a, 0x3f };

struct glb {
	uint8_t debug;
	uint16_t flags;

	char *out_filename;
	char *mac_addr;

};

uint8_t debug = 0;

int parse_parameters( struct glb *g, int argc, char **argv ) {
	return 0;
}

int main( int argc, char **argv ) {
	char cmd[1024];
	char minmax[128];
	char mmode[128];
	char uprefix[128];
	char units[128];
	uint8_t dps = 0;

	FILE *fp, *fo;

	snprintf(cmd,sizeof(cmd), "gatttool -b %s --char-read --handle 0x2d --listen", argv[1]);

	fp = popen(cmd,"r");
	if (fp == NULL) {
		fprintf(stderr,"Error executing '%s'\n", cmd);
		exit(1);
	} else {
		fprintf(stdout,"Success (%s)\n", cmd);
	}

	fo = fopen("owon.txt","w");
	if (fo == NULL) {
		fprintf(stderr,"Couldn't open owon.data file to write\n");
	}


	while (fgets(cmd, sizeof(cmd), fp) != NULL) {
		char *p, *q;
		uint8_t d[14];
		uint8_t i = 0;
		double v = 0.0;


//	fprintf(stdout,"%s", cmd);

		p = strstr(cmd, "2e value: ");
		if (!p) continue;

		if (p) p += sizeof("2e value:");
		if (strlen(p) != 43) {
			fprintf(stdout, "\33[2K\r"); // line erase
			fprintf(stdout,"Waiting...");
		   	continue;
		}
		while ( p && *p != '\0' && (i < 14) ) {
			d[i] = strtol(p, &q, 16);
			if (!q) break;
			p = q;
		if (debug)	fprintf(stdout,"[%d](%02x) ", i, d[i]);
			i++;
		}
		if (debug) fprintf(stdout,"\n");

		/** now decode the data in to human-readable 
		 *
		 * first byte is the sign +/-
		 * bytes 1..4 are ASCII char codes for 0000-9999
		 */
//		fprintf(stdout,"%c%c%c%c%c => ", d[0], d[1], d[2], d[3], d[4] );
		v = ((d[1] -'0') *1000) 
			+((d[2] -'0') *100)
			+((d[3] -'0') *10)
			+((d[4] -'0') *1);


		dps = 0;
		switch (d[6]) {
			case 48: break; // no DP at all (integer) 
			case 49: v = v/1000; dps = 3; break;
			case 50: v = v/100; dps = 2; break;
			case 51: v = v/10; dps = 1; break;
//			case 52: v = v/10; break; // something different though depending on mode
		};

		mmode[0] = '\0';
		switch (d[7]) {
			case 0: snprintf(mmode, sizeof(mmode)," "); break; // temperature?
			case 1: snprintf(mmode, sizeof(mmode),"Manual"); break; // ohms-manual
			case 8: snprintf(mmode, sizeof(mmode),"AC-minmax"); break;
			case 9: snprintf(mmode, sizeof(mmode),"AC-manual"); break;
			case 16: snprintf(mmode, sizeof(mmode),"DC-minmax"); break;
			case 17: snprintf(mmode, sizeof(mmode),"DC-manual"); break;
			case 20: snprintf(mmode, sizeof(mmode),"Delta"); break;
			case 32: snprintf(mmode, sizeof(mmode)," "); break;
			case 33: snprintf(mmode, sizeof(mmode),"Auto"); break; // ohms-auto
			case 41: snprintf(mmode, sizeof(mmode),"AC-auto"); break;
			case 49: snprintf(mmode, sizeof(mmode),"DC-auto");
					 if (d[6] == 50) { v *= 10; dps--; }
					 break;
			case 51: snprintf(mmode, sizeof(mmode),"Hold"); break;
			default: snprintf(mmode, sizeof(mmode),"#%d)",d[7]);
		};

		minmax[0] = '\0';
		switch (d[8]) {
			case 16: snprintf(minmax, sizeof(minmax), "min"); break;
			case 32: snprintf(minmax, sizeof(minmax), "max"); break;
			default: minmax[0] = '\0';
		}

		uprefix[0] = '\0';
		switch (d[9]) {
			case 0: if (d[10] == 4) {
					   	v /= 10; dps++;
						snprintf(uprefix,sizeof(uprefix),"n");
					}
					break;

			case 2: snprintf(uprefix, sizeof(uprefix),"duty"); break;
			case 4: snprintf(mmode, sizeof(mmode),"Diode"); break; // DIODE test mode
			case 8: snprintf(uprefix, sizeof(uprefix)," "); break;
			case 16: snprintf(uprefix, sizeof(uprefix),"M"); break;
			case 32: snprintf(uprefix, sizeof(uprefix),"K"); break;
			case 64: snprintf(uprefix, sizeof(uprefix),"m");
					 if ((d[10] == 128)||(d[10] == 64)) {
						 v /= 10; dps++;
					 }
					 break;
			case 128: snprintf(uprefix, sizeof(uprefix),"u"); break;
			default: snprintf(uprefix, sizeof(uprefix), "#%d", d[9]);
		}

		units[0] = '\0';
		switch(d[10]) {
			case 0: snprintf(units,sizeof(units),"%%");
			case 1: snprintf(units,sizeof(units),"'F" ); break;
			case 2: snprintf(units,sizeof(units),"'C" ); break;
			case 4: snprintf(units,sizeof(units),"F"); break;
			case 8: snprintf(units,sizeof(units),"Hz"); break;
			case 16: snprintf(units,sizeof(units),"hFe"); break;
			case 32: snprintf(units,sizeof(units),"Ohm");
					v = v/10;
					 dps++;
					break;
			case 64: snprintf(units,sizeof(units),"A"); break;
			case 128: snprintf(units,sizeof(units),"V"); break;
			default: snprintf(units,sizeof(units),"#%d", d[10]);
		};

		if (d[0] == '-') v = -v; // polarity

		//fprintf(stdout, "%+5.4g units:%s%s mode:%s minmax:%s\n",v, uprefix, units, mmode, minmax);
//		fprintf(stdout, "\33[2K\r"); // line erase
//		fprintf(stdout, "\x1B[A"); // line up
//		fprintf(stdout, "\33[2K\r"); // line erase
//		fprintf(stdout, "\x1B[A"); // line up
		//fprintf(stdout, "\33[2K\r"); // line erase

		/** range checks **/
		if ( memcmp( d, OLs, 5 ) == 0 )  {
			snprintf(cmd, sizeof(cmd), "O.L %s\n%s", units, mmode );
		} else {
			if (dps < 0) dps = 0;
			if (dps > 4) dps = 4;

			switch (dps) {
				case 0: snprintf(cmd, sizeof(cmd), "%+05.0f%s%s\n%s %s",v, uprefix, units, mmode, minmax); break;
				case 1: snprintf(cmd, sizeof(cmd), "%+06.1f%s%s\n%s %s",v, uprefix, units, mmode, minmax); break;
				case 2: snprintf(cmd, sizeof(cmd), "%+06.2f%s%s\n%s %s",v, uprefix, units, mmode, minmax); break;
				case 3: snprintf(cmd, sizeof(cmd), "%+06.3f%s%s\n%s %s",v, uprefix, units, mmode, minmax); break;
				case 4: snprintf(cmd, sizeof(cmd), "%+06.4f%s%s\n%s %s",v, uprefix, units, mmode, minmax); break;
			}
		}
//		fprintf(stdout, "\x1B[A"); // line up
		if (fo) {
				rewind(fo);
				fprintf(fo, "%s%c", cmd, 0);
				fflush(fo);
		}

		fprintf(stdout, "\33[2K\r"); // line erase
		fprintf(stdout, "\x1B[A"); // line up
		fprintf(stdout, "\33[2K\r"); // line erase
		fprintf(stdout,"%s",cmd);
		fflush(stdout);
		
	}

	if (pclose(fp)) {
		fprintf(stdout,"Command not found, or exited with error\n");
	}
	return 0;
}
