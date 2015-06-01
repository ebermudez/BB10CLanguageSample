/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * Parser for the news shown in the home view.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#include "SoapParserNoticiasHome.hpp"

SoapParserNoticiasHome::SoapParserNoticiasHome() {
	// TODO Auto-generated constructor stub

}

SoapParserNoticiasHome::~SoapParserNoticiasHome() {
	// TODO Auto-generated destructor stub
}

QList<NoticiaHomeWeb*>* SoapParserNoticiasHome::getLista() {
	return Lista;
}

void SoapParserNoticiasHome::read(QByteArray buffer_reply){

       QXmlStreamReader *xml = new QXmlStreamReader(buffer_reply);

       Lista = new QList<NoticiaHomeWeb*>;

       NoticiaHomeWeb *noticia;

       while (!xml->atEnd() && !xml->hasError())
                 {

                    xml->readNext();

                     if (xml->isStartElement())
                     {

                    	 QString name = xml->name().toString();

                          if(name == "Noticia"){
                                 noticia = new NoticiaHomeWeb();
                                 noticia->setId(xml->attributes().value("Id").toString());
                                 noticia->setTitulo(xml->attributes().value("Titulo").toString());
                                 noticia->setAutor(xml->attributes().value("Autor").toString());
                                 noticia->setSeccion(xml->attributes().value("Seccion").toString());
                                 noticia->setTipo(xml->attributes().value("Tipo").toString());
                                 noticia->setUrlImagen(xml->attributes().value("UrlImagen").toString());
                                 noticia->setUrlImagenGrande(xml->attributes().value("UrlImagenGrande").toString());
                                 noticia->setUrlNoticia(xml->attributes().value("UrlNoticia").toString());


                                 QString fechaFormateada = xml->attributes().value("Fecha").toString();


                                 if (fechaFormateada.contains("T"))
                                 {
                                	 QStringList qlist1 = xml->attributes().value("Fecha").toString().split("T");


                                	 if (qlist1.size() > 0 && qlist1.at(0).contains("-"))
                                	 {
                                		 QStringList qlist2 = qlist1.at(0).split("-");

                                		 if (qlist2.size() == 3)
                                		 {
                                			 fechaFormateada = QString("");
                                			 fechaFormateada.append(qlist2.at(2));
                                			 fechaFormateada.append(QString("-"));
                                			 fechaFormateada.append(qlist2.at(1));
                                			 fechaFormateada.append(QString("-"));
                                			 fechaFormateada.append(qlist2.at(0));
                                		 }
                                	 }
                                 }
                                 noticia->setFecha(fechaFormateada);
                                 noticia->setSumario(xml->attributes().value("Sumario").toString());
                                 noticia->setContenido(xml->attributes().value("Contenido").toString());
                                 noticia->setContenidoLimpio(xml->attributes().value("ContenidoLimpio").toString());
                          }

                     }
                     else if (xml->isEndElement())
                     {
                    	 QString name = xml->name().toString();

                    	 if(name == "Noticia") {
                    		 Lista->append(noticia);
                    	 }

                     }
                     else if (xml->hasError())
                     {

                     }
                     else if (xml->atEnd())
                     {

                     }

                 }

}
