
function $(id)
{
    return document.getElementById(id);
}
function nodeScriptReplace(node) 
{
    if ( nodeScriptIs(node) === true ) 
    {
        node.parentNode.replaceChild( nodeScriptClone(node) , node );
    }
    else 
    {
        var i = -1, children = node.childNodes;
        while ( ++i < children.length ) 
        {
                nodeScriptReplace( children[i] );
        }
    }  
    return node;
}
function nodeScriptClone(node)
{
        var script  = document.createElement("script");
        script.text = node.innerHTML;
        var i = -1, attrs = node.attributes, attr;
        while ( ++i < attrs.length ) 
        {                                    
            script.setAttribute( (attr = attrs[i]).name, attr.value );
        }
        return script;
}

function nodeScriptIs(node) 
{
        return node.tagName === 'SCRIPT';
}



class CantorPanelImpl extends HTMLElement  
{
    ContentInsideFullHtmlDoc = false;
    static get observedAttributes() { return ['src']; }

    attributeChangedCallback(attrName, oldVal, newVal) 
    {
        this[attrName] = newVal;
        if(attrName =="src")
        {
            this.loadFromContent();
        }
    }
    loadFromContent()
    {
        $("test")
        let self = this;

        function reqListener () 
        {
            let content = this.responseText;
            if(self.ContentInsideFullHtmlDoc)
            {
                self.innerHTML = content;
                nodeScriptReplace(self);
            }
            else
            {
                self.innerHTML = content;
            }

        }
        let docUrl = window.location.href
        let url = docUrl.substring( 0, docUrl.lastIndexOf( "/" ) + 1);
        let src_url = this.getAttribute('src');
        if(src_url.endsWith(".html"))
        {
            this.ContentInsideFullHtmlDoc = true;
        }
        let full_url = url+src_url
        var oReq = new XMLHttpRequest();
        oReq.addEventListener("load", reqListener);
        oReq.open("GET", full_url);
        oReq.send();
    }
    connectedCallback()
    {
        this.loadFromContent();
    }

}
customElements.define('cantor-panel', CantorPanelImpl);
                    