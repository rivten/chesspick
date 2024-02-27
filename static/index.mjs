import { h, render, Component } from './preact.mjs';
import htm from './htm.mjs';

const html = htm.bind(h);

class Login extends Component {
    constructor() {
        super();
        this.state = {
            isLoggedIn: document.cookie.indexOf("chesspick_loggedin=1") !== 1,
        }
    }

    render() {
        if (this.state.isLoggedIn) {
            return html`<p>Logged in</p>`
        } else {
            return html`
                <form action="/api/login" method="get">
                    <input type="submit" value="Login" />
                </form>
            `
        }
    }
}

class App extends Component {

    onClick = async ev => {
        const pick = ev.target.getAttribute("data-pick");
        if (pick === undefined) {
            console.error("invalid pick");
        } else {
            const url = "/api/pick";
            const response = await fetch(url, {
                method: "POST",
                body: pick,
            })
            console.log(await response.json())
        }

        ev.preventDefault()
    }

    render() {
        return html`
            <h1>Hello !</h1>
            <button type="button" onClick=${this.onClick} data-pick="white">Pick white</button>
            <button type="button" onClick=${this.onClick} data-pick="black">Pick black</button>
            <${Login} />
        `;
    }
}

render(html`<${App} />`, document.body);
