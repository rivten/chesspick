import { h, render, Component } from './preact.mjs';
import htm from './htm.mjs';

const html = htm.bind(h);

class App extends Component {
    state = {}

    onClick = async ev => {
        const pick = ev.target.getAttribute("data-pick");
        if (pick === undefined) {
            console.error("invalid pick");
        } else {
            const url = "/api/pick";
            const response = await fetch(url, {
                method: "POST",
                body: pick,
            });
            console.log(response.json());
        }

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
