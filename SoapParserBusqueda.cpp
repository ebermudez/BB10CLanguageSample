/**
 * Author: Edgardo Bermúdez
 * This class was modified to be able to be shared online via GitHub. 
 * Parser for searched items.
 * 
 * BlackBerry 10 - C++ and QML
 * 
 */

#include "SoapParserBusqueda.hpp"

SoapParserBusqueda::SoapParserBusqueda() {
	// TODO Auto-generated constructor stub

}

SoapParserBusqueda::~SoapParserBusqueda() {
	// TODO Auto-generated destructor stub
}

QList<NoticiaHomeWeb*>* SoapParserBusqueda::getLista() {
	return Lista;
}

void SoapParserBusqueda::setTotal_Resultados(int total){
	this->total_Resultados = total;
}

int SoapParserBusqueda::getTotal_Resultados() const{
	return total_Resultados;
}

void SoapParserBusqueda::setTotal_Paginas(int total){
	this->total_Paginas = total;
}

int SoapParserBusqueda::getTotal_Paginas() const{
	return total_Paginas;
}

void SoapParserBusqueda::read(QByteArray buffer_reply){

	QXmlStreamReader *xml = new QXmlStreamReader(buffer_reply);

	Lista = new QList<NoticiaHomeWeb*>;

	NoticiaHomeWeb *noticia;

	while (!xml->atEnd() && !xml->hasError())
	{

		xml->readNext();

		if (xml->isStartElement())
		{

			QString name = xml->name().toString();

			if(name == "BusquedaArticulos") {
				setTotal_Paginas(xml->attributes().value("Total_Paginas").toString().toInt());
				setTotal_Resultados(xml->attributes().value("Total_Resultados").toString().toInt());
			}else if(name == "Articulo"){
				noticia = new NoticiaHomeWeb();
				noticia->setId(xml->attributes().value("Id").toString());
				noticia->setTitulo(xml->attributes().value("Titulo").toString());
				noticia->setFecha(xml->attributes().value("FechaCreacion").toString());
				noticia->setUrlNoticia(xml->attributes().value("UrlArticulo").toString());
				noticia->setUrlImagen(xml->attributes().value("UrlImagen").toString());
				noticia->setUrlImagenGrande(xml->attributes().value("UrlImagen").toString());
				noticia->setSumario(xml->attributes().value("Sumario").toString());
				noticia->setSeccion(xml->attributes().value("Seccion").toString());

			}

		}
		else if (xml->isEndElement())
		{
			QString name = xml->name().toString();

			if(name == "Articulo") {
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
