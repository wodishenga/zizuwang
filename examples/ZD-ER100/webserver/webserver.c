/*
 * Copyright (c) 2017, RISE SICS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "contiki.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/ipv6/uip-sr.h"

#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
//static const char *TOP = "<html>\n  <head>\n    <title>Contiki-NG</title>\n  </head>\n<body>\n";
//static const char *BOTTOM = "\n</body>\n</html>\n";



static const char *TOP = " <!DOCTYPE html>\n <html>\n  <head>\n\
		  <meta charset=\"UTF-8\">\n\
		  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
		  <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\">\n\
		  <title>Contiki-NG-Fanleung</title>\n <style>\n\
* {\n\
  padding: 0;\n\
  margin: 0;\n\
}\n\
html,\n\
body {\n\
  width: 100%;\n\
  height: 98%;\n\
}\n\
svg {\n\
  width: 100%;\n\
  height: 98%;\n\
}\n\
.links line { \
  stroke: rgb(86, 87, 87); \
  stroke-opacity: 1; \
} \
.nodes circle { \
  stroke: #fff; \
  stroke-width: 1.5px; \
}		 \
text {\
  font-family: sans-serif;\
  font-size:16px;\
}\
</style>\n  </head>\n <body>\n\
<svg width=\"1400\" height=\"950\"></svg>\n\
<script src=\"https://d3js.org/d3.v4.min.js\"></script>\n\
<script>\n\
var svg = d3.select(\"svg\"),\
    width = +svg.attr(\"width\"),\
    height = +svg.attr(\"height\");\
console.log(svg)\n\
var color = d3.scaleOrdinal(d3.schemeCategory20);\
var simulation = d3.forceSimulation()\
    .force(\"link\", d3.forceLink().id(function(d) { return d.id; }).distance(110))\
    .force(\"charge\", d3.forceManyBody().strength(-100))\
    .force(\"center\", d3.forceCenter((width / 2 + 20), height / 2));\n";



/////////////////////////////
static const char *BOTTOM = "var link = svg.append(\"g\")\n\
    .attr(\"class\", \"links\")\n\
    .selectAll(\"line\")\n\
    .data(graph.links)\n\
    .enter().append(\"line\")\n\
    .attr(\"stroke-width\", function(d) { return 3; });\n\
var node = svg.append(\"g\")\n\
  .attr(\"class\", \"nodes\")\n\
  .selectAll(\"g\")\n\
  .data(graph.nodes)\n\
  .enter().append(\"g\");\n\
var circles = node.append(\"circle\")\n\
  .attr(\"r\", 10)\n\
  .attr(\"fill\", function(d) { return color(d.group); })\n\
  .call(d3.drag()\n\
  .on(\"start\", dragstarted)\n\
  .on(\"drag\", dragged)\n\
  .on(\"end\", dragended));\n\
var lables = node.append(\"text\")\n\
  .text(function(d) {\n\
	return d.id;\n\
  })\n\
  .attr('x', 10)\n\
  .attr('y', 5);\n\
node.append(\"title\")\n\
  .text(function(d) { return d.id; });\n\
simulation\n\
  .nodes(graph.nodes)\n\
  .on(\"tick\", ticked);\n\
simulation.force(\"link\")\n\
  .links(graph.links);\n\
function ticked() {\n\
  link\n\
	.attr(\"x1\", function(d) { return d.source.x; })\n\
	.attr(\"y1\", function(d) { return d.source.y; })\n\
	.attr(\"x2\", function(d) { return d.target.x; })\n\
	.attr(\"y2\", function(d) { return d.target.y; });\n\
node\n\
	.attr(\"cx\", function(d) {\n\
	return d.x = Math.max(10, Math.min(width-10, d.x));\n\
	})\n\
	.attr(\"cy\", function(d) {\n\
	return d.y = Math.max(16, Math.min(height-16, d.y));\n\
	})\n\
	.attr(\"transform\", function(d) {\n\
	  return \"translate(\" + d.x + \",\" + d.y + \")\";\n\
	})\n\
}\n\
function dragstarted(d) {\n\
  if (!d3.event.active) simulation.alphaTarget(0.3).restart();\n\
  d.fx = d.x;\n\
  d.fy = d.y;\n\
}\n\
function dragged(d) {\n\
  d.fx = d3.event.x;\n\
  d.fy = d3.event.y;\n\
}\n\
function dragended(d) {\n\
  if (!d3.event.active) simulation.alphaTarget(0);\n\
  d.fx = null;\n\
  d.fy = null;\n\
}\n\
</script>\n\n";


static char buf[1024];
static int blen;
#define ADD(...) do {                                                   \
    blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);      \
  } while(0)
#define SEND(s) do { \
  SEND_STRING(s, buf); \
  blen = 0; \
} while(0);

/* Use simple webserver with only one page for minimum footprint.
 * Multiple connections can result in interleaved tcp segments since
 * a single static buffer is used for all segments.
 */
#include "httpd-simple.h"

/*---------------------------------------------------------------------------*/
static void
ipaddr_add(const uip_ipaddr_t *addr)
{
  uint16_t a;
  int i, f;
  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        ADD("::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        ADD(":");
      }
      ADD("%x", a);
    }
  }
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_routes(struct httpd_state *s))
{
//  static uip_ds6_nbr_t *nbr;

  uip_ipaddr_t root_ipaddr;
  NETSTACK_ROUTING.get_root_ipaddr(&root_ipaddr);

  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, TOP);

//  ADD("  Neighbors\n  <ul>\n");

  ADD("var graph = {\"nodes\": [");
  ADD("{\"id\": \"");
  ipaddr_add(&root_ipaddr);
  ADD("\", \"group\": 1},\n");
  SEND(&s->sout);

  if(uip_sr_num_nodes() > 0) {
    static uip_sr_node_t *link;
    for(link = uip_sr_node_head(); link != NULL; link = uip_sr_node_next(link)) {
      if(link->parent != NULL) {
        uip_ipaddr_t child_ipaddr;
        NETSTACK_ROUTING.get_sr_node_ipaddr(&child_ipaddr, link);

		ADD("{\"id\": \"");
        ipaddr_add(&child_ipaddr);
        ADD("\", \"group\": 1},\n");
        SEND(&s->sout);
      }
    }
    ADD("],\n");
    SEND(&s->sout);
  }

//  for(nbr = uip_ds6_nbr_head();
//      nbr != NULL;
//      nbr = uip_ds6_nbr_next(nbr)) {
//
//    ADD("{\"id\": \"");
//    nbr->ipaddr.u8[0] = 0xfD;
//    nbr->ipaddr.u8[1] = 0x00;
//    ipaddr_add(&nbr->ipaddr);
//    ADD("\", \"group\": 1},\n");
//    SEND(&s->sout);
//  }
//  ADD(" ],");
//  SEND(&s->sout);



#if (UIP_MAX_ROUTES != 0)
//  {
//    static uip_ds6_route_t *r;
//    ADD("  Routes\n  <ul>\n");
//    SEND(&s->sout);
//    for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
//      ADD("    <li>");
//      ipaddr_add(&r->ipaddr);
//      ADD("/%u (via ", r->length);
//      ipaddr_add(uip_ds6_route_nexthop(r));
//      ADD(") %lus", (unsigned long)r->state.lifetime);
//      ADD("</li>\n");
//      SEND(&s->sout);
//    }
//    ADD("  </ul>\n");
//    SEND(&s->sout);
//  }
#endif /* UIP_MAX_ROUTES != 0 */



#if (UIP_SR_LINK_NUM != 0)
  if(uip_sr_num_nodes() > 0) {
    static uip_sr_node_t *link;
    ADD(" \"links\":[");
    SEND(&s->sout);
    for(link = uip_sr_node_head(); link != NULL; link = uip_sr_node_next(link)) {
      if(link->parent != NULL) {
        uip_ipaddr_t child_ipaddr;
        uip_ipaddr_t parent_ipaddr;

        NETSTACK_ROUTING.get_sr_node_ipaddr(&child_ipaddr, link);
        NETSTACK_ROUTING.get_sr_node_ipaddr(&parent_ipaddr, link->parent);

        ADD(" {\"source\": \"");
        ipaddr_add(&child_ipaddr);

        ADD("\", \"target\":\"");
        ipaddr_add(&parent_ipaddr);
//        ADD(") %us", (unsigned int)link->lifetime);
        ADD("\",\"value\":1},\n");
        SEND(&s->sout);
      }
    }
    ADD("]};\n");
    SEND(&s->sout);
    SEND_STRING(&s->sout, BOTTOM);
    ADD("<h1>当前已组网节点数: %u</h1></body>\n </html>", uip_sr_num_nodes());
    SEND(&s->sout);
  }
#endif /* UIP_SR_LINK_NUM != 0 */

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
PROCESS(webserver_nogui_process, "Web server");
PROCESS_THREAD(webserver_nogui_process, ev, data)
{
  PROCESS_BEGIN();

  httpd_init();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
httpd_simple_script_t
httpd_simple_get_script(const char *name)
{
  return generate_routes;
}
/*---------------------------------------------------------------------------*/
