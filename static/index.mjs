import { h, render, Component } from './preact.mjs';
import htm from './htm.mjs';

const html = htm.bind(h);

class App extends Component {
    state = {}

    onClick = ev => {
        console.log(ev.target.getAttribute("data-pick"));
        ev.preventDefault()
    }

    render() {
        return html`
            <h1>Hello !</h1>
            <button type="button" onClick=${this.onClick} data-pick="white">Pick white</button>
            <button type="button" onClick=${this.onClick} data-pick="black">Pick black</button>
        `;
    }
}

render(html`<${App} />`, document.body);
