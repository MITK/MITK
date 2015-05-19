<?xml version="1.0" encoding="UTF-8" ?>

<!-- New document created with EditiX at Thu Feb 02 20:41:19 CET 2012 -->

<xsl:stylesheet version="2.0" 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:fn="http://www.w3.org/2005/xpath-functions"
	xmlns:xdt="http://www.w3.org/2005/xpath-datatypes"
	xmlns:err="http://www.w3.org/2005/xqt-errors"
	exclude-result-prefixes="xs xdt err fn">

	<xsl:output method="xml" indent="yes"/>
	
	
<!-- Skip the first section element which represents the "Related Pages" entry -->
	
<xsl:template match="toc/section">
		<xsl:apply-templates/>
	
</xsl:template>
	
	
<xsl:template match="node()|@*">
		
<xsl:copy><xsl:apply-templates select="node()|@*"/></xsl:copy>
	
</xsl:template>
	
	<xsl:template match="/">
		<xsl:apply-templates/>
	</xsl:template>

</xsl:stylesheet>
