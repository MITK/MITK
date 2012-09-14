  <!--
  =========================================================================================
    For an input image, we use QmitkDataStorageComboBoxWithSelectNone and no Browse button.
    But for output image, we use standard ctkPathLineEdit which has its own browse button.
  =========================================================================================
  -->
  <xsl:template match="parameters/*[name()=('image')]" priority="2">
    <xsl:call-template name="gridItemWithLabel"/>
    <item  row="{position()-1}" column="1">
      <layout class="QHBoxLayout">
      <xsl:choose>
        <xsl:when test="channel = 'input'">
          <item>
            <widget class="{$imageInputWidget}"  name="parameter:{name}">
              <xsl:call-template name="commonWidgetProperties"/>
              <xsl:call-template name="createQtDesignerStringListProperty"/>
              <property name="parameter:valueProperty"> <!-- property name containing current value -->
                <string>currentValue</string>
              </property>
            </widget>
          </item>
        </xsl:when>
        <xsl:otherwise>
          <item>
            <widget class="{$imageOutputWidget}"  name="parameter:{name}">
              <xsl:call-template name="commonWidgetProperties"/>
              <xsl:call-template name="createQtDesignerStringListProperty"/>
              <property name="filters">
                <set>ctkPathLineEdit::Files,ctkPathLineEdit::Writable</set>
              </property>
            </widget>
          </item>
        </xsl:otherwise>
      </xsl:choose>
      </layout>
    </item>
  </xsl:template>
