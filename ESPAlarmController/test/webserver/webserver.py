#!/usr/bin/env python3
import cherrypy
import os


class Root(object):
    @cherrypy.expose
    @cherrypy.tools.allow(methods=['GET'])
    def index(self):
        with open('../../data/html/index.html', 'rt') as webpage:
            return webpage.read()
    

if __name__ == "__main__":
    conf = {
        '/': {
            'tools.staticdir.on': True,
            'tools.staticdir.dir': os.path.abspath('../../data/html/')
        }
    }

    cherrypy.config.update({'server.socket_port': 8087})
    cherrypy.server.socket_host = '0.0.0.0'
    cherrypy.quickstart(Root(), '/', conf)
